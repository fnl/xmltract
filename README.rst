xmltract
========

Overview
--------

Installation::

    make
    ./xmltract -p ns val test.xml # should print "foo inner bar"
    cp xmltract /usr/local/bin/
    make clean

Usage::

    $ xmltract -h
    usage: xmltract [-hiqv] [-e encoding] [-p prefix] name [infiles]
    
    extract content for a particular element (name) from XML
    
    -h      print this help and exit
    -i      ignore case of name (and prefix)
    -q      quiet logging (errors only)
    -v      verbose logging (default: warnings)
    -e ENC  set encoding (default: UTF-8)
    -p PFX  match prefix, too

    $ # count uniqe element content
    $ xmltract val test.xml | sort | uniq -c | sort -n
    1 foo inner bar
    1 inner

    $ # extract a particular namespace prefix only and ignore case
    $ xmltract -i -p NS2 VAL test.xml
    inner

    $ # extract content from an XML stream or an (X)HTML page
    $ curl http://www.w3schools.com/xml/cd_catalog.xml | xmltract TITLE
    ... (prints all CD titles in cd_catalog.xml) ...

Notes
-----

All content will be normalized by trimming spaces and normalizing successive spaces to single whitespaces. Content includes any content found within children, too: If an element is defined recursively within itself, it will be exactracted as two independent content strings::

    <NAME>
    Hi there,
    <NAME>Bob</NAME>
    </NAME>

Extracting ``NAME`` from the above example will result in::

    Hi there, Bob
    Bob

| Author: Florian Leitner (c) 2012
| License: Public Domain
| Disclaimer: Use at your own risk and sole responsibility, without warranties or conditions of any kind.
| Note: This is derative work based on GLib (LGPL) and Libxml2 (MIT License).
