import asyncio
import calendar
import datetime as dt
import sys
import time
from dataclasses import dataclass, field
from urllib.parse import urljoin

import aiohttp
from aiofile import async_open
from aiopath import AsyncPath
from aiostream import async_, operator, pipe, stream, streamcontext
from bs4 import BeautifulSoup


def apartial(fn, *args, **kwargs):
    """Partially applies args and kwargs to a async function. It immitates
    `functools.partial`."""
    return async_(
        lambda *args2, **kwargs2: fn(
            *args,
            *args2,
            **kwargs,
            **kwargs2,
        ),
    )


@dataclass
class Article:
    """Gathers basic information of an article in a magazine."""

    date: dt.date = field(init=False)
    title: str
    magazine_url: str
    url: str

    def __post_init__(self):
        """Sets article date based on URL."""

        path_split = self.magazine_url.split("/")
        month_names = list(map(str.lower, calendar.month_name))

        if len(month := path_split[-1].split("-")) == 1:
            month, day = month[0], 1
        else:
            month, day = month[0], int(month[1])

        month_num = month_names.index(month)
        try:
            year = int(path_split[-3].split("-")[-1])
        except ValueError:
            year = int(path_split[-2].split("-")[-1])

        self.date = dt.date(year=year, month=month_num, day=day)

    async def content(self) -> asyncio.StreamReader:
        """Gets article content in textual form returning an async reader. It
        uses `links` to query readable text without markup from the article's
        link."""

        retry_sleep_base = 0.1  # in seconds
        max_retries = 10
        tried = 0
        while tried < max_retries:
            await asyncio.sleep(retry_sleep_base * (tried ** 2))
            ps = await asyncio.create_subprocess_exec(
                "links",
                "-dump",
                self.url,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )
            await ps.wait()
            if ps.returncode == 0:
                break
            tried += 1

        if ps.returncode != 0:
            raise ValueError(f"Failed to query article '{self.title}' ({self.url})")
        return ps.stdout


async def fetch_magazine(session, url):
    """Fetches all articles in a magazine, returning them on an asynchronous
    stream."""

    async with session.get(
        url,
        headers={"User-Agent": ""},
        allow_redirects=False,
    ) as resp:
        resp.raise_for_status()
        # ignores redirects, for they sign magazine for this month doesn't
        # exist.
        if 400 > resp.status >= 300:
            return stream.iterate(tuple())

        bs = BeautifulSoup(await resp.read(), "html.parser")

        articles = []
        for a_tag in bs.find("article").select("a.lnk"):
            study, title = None, None

            article_link = urljoin(url, a_tag["href"])

            if card_line1 := a_tag.find("div", class_="cardLine1"):
                study = card_line1.text.strip()
            if card_line2 := a_tag.find("div", class_="cardLine2"):
                title = card_line2.text.strip()

            if study and not title:
                title = study
            else:
                title = f"{study}-{title}"

            articles.append(
                Article(
                    title=title,
                    magazine_url=url,
                    url=article_link,
                )
            )

        return stream.iterate(articles)


async def fetch_month(session, year: int, month: int):
    """Fetches all articles in all magazines of a month, returning them on an
    asynchronous stream."""

    wts_url = "https://wol.jw.org/en/wol/library/r1/lp-e/all-publications/watchtower/"
    recent_year_wts_url = urljoin(wts_url, f"the-watchtower-{year}/study-edition/")
    old_year_wts_url = urljoin(wts_url, f"the-watchtower-{year}/")

    month_name = calendar.month_name[month].lower()
    if year >= 2016:
        month_wt_urls = (urljoin(recent_year_wts_url, month_name),)
    elif 2015 >= year >= 2008:
        month_wt_urls = (urljoin(recent_year_wts_url, month_name + "-15"),)
    elif 2007 >= year:
        month_wt_urls = (
            urljoin(old_year_wts_url, month_name + "-15"),
            urljoin(old_year_wts_url, month_name + "-1"),
        )

    return stream.flatten(
        stream.iterate(month_wt_urls) | pipe.map(apartial(fetch_magazine, session))
    )


async def fetch_year(session, year):
    """Fetches all articles in all magazines of a year, returning them on an
    asynchronous stream."""
    return stream.flatten(
        stream.range(1, 13) | pipe.map(apartial(fetch_month, session, year))
    )


async def save_article(base_path: str, article: Article):
    """Saves an article on disk given a base path. The file should be saved on
    `<base_path>/<year>/<year>-<month>-<day-<title>.txt`."""
    base_path = AsyncPath(base_path)
    date = article.date
    year_path = base_path / str(date.year)
    await year_path.mkdir(parents=True, exist_ok=True)

    article_filename = f"{date.year}-{date.month:02}-{date.day:02}-{article.title}.txt"
    article_path = year_path / article_filename

    async with async_open(article_path, "wb") as file:
        content = await article.content()
        await file.write(await content.read())

    return article


@operator(pipable=True)
async def streamprogress(source):
    """Logs stream progress as records are processed and returns the number of
    records processed.."""
    count = 0
    async with streamcontext(source) as streamer:
        async for _ in streamer:
            sys.stdout.write(".")
            sys.stdout.flush()
            count += 1
    yield count


async def main(base_path: str, from_year: int, to_year: int):
    """Fetches all articles from a year range and saves them on disk, displaying
    operations progress."""

    async with aiohttp.ClientSession() as session:
        saved = await (
            stream.flatten(
                stream.map(
                    stream.range(from_year, to_year),
                    apartial(fetch_year, session),
                ),
            )
            | pipe.map(apartial(save_article, base_path))
            | streamprogress.pipe()
        )
    print(f"\nsaved {saved} articles")


if __name__ == "__main__":
    base_path = "/ext/media/articles/wt/"
    step = 2
    # just some simple throttling, since wol.jw.org will refuse requests if it
    # detects too many requests.
    for from_year in range(1950, 2023, step):
        to_year = from_year + step
        print(f"querying articles from years {from_year}-{to_year}")
        asyncio.run(main(base_path, from_year, to_year))
        time.sleep(10)
