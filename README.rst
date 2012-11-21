xmltract
========

Overview
--------

Requirements:

* GLib_
* Libxml2_
* pkg-config_

Installation::

    make
    ./xmltract -p ns val test.xml # should print "foo inner bar"
    mv xmltract /usr/local/bin/

Usage::

    $ xmltract -h
    usage: xmltract [-hiqv] [-e encoding] [-p prefix] name [infiles]
    ... (prints help string) ...

    $ # count uniqe character content in elements
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

All character content will be normalized by trimming spaces and normalizing successive **spaces** to single **whitespaces**. Content includes any content found within children. If an element is defined recursively within itself, they will be extracted as independent content strings::

    <NAME>
    Hi there,
    <NAME>Bob</NAME>
    </NAME>

Extracting the character data from the ``NAME`` elements in the above example will result in::

    Hi there, Bob
    Bob

Author
  Florian Leitner (c) 2012

License
  Public Domain

Disclaimer
  Use at your own risk and sole responsibility, without warranties or conditions of any kind.

Note
  This is derative work based on GLib_ (LGPL) and Libxml2_ (MIT License).

.. _pkg-config: http://pkgconfig.freedesktop.org/
.. _GLib: http://library.gnome.org/
.. _Libxml2: http://xmlsoft.org/
