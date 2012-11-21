xmltract
========

Overview
--------

Installation::

    make
    cp xmltract /usr/local/bin/
    make clean

Usage::

    # show a count of uniqe element content
    xmltract elementName *.xml | sort | uniq -c | sort -n
    # continously extract content from a stream
    curl -s http://www.w3schools.com/xml/cd_catalog.xml | xmltract TITLE
    # (prints all CD titles in the catalog)

Notes
-----

All content will be normalized by trimming spaces and normalizing successive spaces to single whitespaces. Content includes any content found within in children, too. If an element is defined recursively within itself, it will be exactracted as two independent content strings::

    <NAME>
    Hi there,
    <NAME>Bob</NAME>
    </NAME>

Extracting ``NAME`` from the above example will result in::

    Hi there, Bob
    Bob

Legal BS
--------

| Author: Florian Leitner (c) 2012
| License: Public Domain
| Disclaimer: Use at your own risk and responsibility.
