#!/usr/bin/env python3

import html
import re
import sys
import urllib.parse
import urllib.request

PROGRAMME_URL = "https://www.svtplay.se/aktuellt"
BASE_URL = "https://www.svtplay.se"


def fetch_page(url):
    request = urllib.request.Request(
        url,
        headers={
            "User-Agent": "Mozilla/5.0"
        }
    )

    with urllib.request.urlopen(request, timeout=20) as response:
        return response.read().decode("utf-8", errors="replace")


def clean_episode_url(url):
    url = html.unescape(url)

    # Convert relative SVT URL to full URL.
    full_url = urllib.parse.urljoin(BASE_URL, url)

    # Remove query parameters such as ?video=visa.
    parsed = urllib.parse.urlsplit(full_url)
    cleaned = urllib.parse.urlunsplit((
        parsed.scheme,
        parsed.netloc,
        parsed.path,
        "",
        ""
    ))

    return cleaned


def find_top_area_play_button_url(page_html):
    # Find an <a> tag that contains data-rt="top-area-play-button"
    # and extract its href.
    anchor_pattern = r'<a\b(?=[^>]*data-rt="top-area-play-button")[^>]*>'
    href_pattern = r'href="([^"]+)"'

    anchor_match = re.search(anchor_pattern, page_html)

    if anchor_match is None:
        return None

    anchor_tag = anchor_match.group(0)
    href_match = re.search(href_pattern, anchor_tag)

    if href_match is None:
        return None

    return clean_episode_url(href_match.group(1))


def main():
    page_html = fetch_page(PROGRAMME_URL)

    episode_url = find_top_area_play_button_url(page_html)

    if episode_url is None:
        print("ERROR: Could not find top-area play button URL", file=sys.stderr)
        sys.exit(1)

    print(episode_url)


if __name__ == "__main__":
    main()