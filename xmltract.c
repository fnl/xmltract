/*!
 * \file       xmltract.c
 * \brief      Iterative extraction of a particular XML element's content
 *             selected by name (and prefix).
 * \details    This is derative work requiring the Libxml2 (MIT License).
 *
 * \author     Florian Leitner
 * \copyright  MIT License, 2012
 * \warning    Use at your own risk and sole responsibility,
 *             without warranties or conditions of any kind.
 */

#include <libgen.h> // basename()
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

/** Ensure the two strings are both NULL or match.  */
int match(const char *str1, const char *str2) {
  if (str1 != str2) {
    if (str1 == NULL || str2 == NULL) return 0;
    else if (strcmp(str1, str2) == 0) return 1;
    else return 0; 
  }

  // same pointer (incl. both NULL)
  return 1;
}

/** Trim spaces and normalize successive spaces to a single whitespace. */
size_t trim(char *out, size_t len, const char *str) {
  *out = 0;

  if (len == 0) return 0;

  const char *end;
  size_t out_size = 0;
  int space = 0;

  // trim leading spaces
  while (isspace(*str)) str++;

  // if not all spaces
  if (*str != 0) {
    *out = *str;
    out_size = 1;

    // trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    // copy trimmed string
    while (str++ != end)
      // ignore successive spaces and normlize to whitespace
      if (isspace(*str)) {
        if (space == 0) {
          out[out_size++] = ' ';
          space = 1;
        }
      } else {
        out[out_size++] = *str;
        space = 0;
      }

    // add null terminator
    out[out_size] = 0;
  }

  return out_size;
}

/** Convert a string to uppercase (in memmory). */
void strtoupper(char *str) {
  if (str != NULL)
    for (; *str != 0; str++)
      *str = toupper(*str);
}

/** Extract the contents of a particular XML element (name) from a file input stream. */
int parse(xmlTextReaderPtr reader, const char *prefix, const char *name, int ignore_case) {
  int result;
  char *node_name;
  char *node_prefix = NULL;
  char *content;
  size_t len;

  // keep iterating over elments in the stream
  while ((result = xmlTextReaderRead(reader)) == 1) {
    if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_ELEMENT) {
      // compare the element's (prefix and) name to the target name
      node_name = (char *) xmlTextReaderLocalName(reader);

      if (ignore_case) strtoupper(node_name);

      if (prefix) {
        node_prefix = (char *) xmlTextReaderPrefix(reader);
        if (ignore_case) strtoupper(node_prefix);
      }

      if (match(prefix, node_prefix) && match(name, node_name)) {
        // print a matching element's content as a trimmed string
        content = (char *) xmlTextReaderReadString(reader);

        if (content) {
          len = strlen(content);
          char trimmed[len];
          if (trim(trimmed, len, content) > 0) puts(trimmed);
        }
      }
    }
  }

  return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/** Extract the particular XML content from a file. */
int parseFile(
    const char *filename,
    const char *encoding,
    const char *prefix,
    const char *name,
    int ignore_case) {
  xmlTextReaderPtr reader = xmlReaderForFile(filename, encoding, 0);
  int parse_result = -1;

  // parse the file if the reader could be created
  if (reader) {
    parse_result = parse(reader, prefix, name, ignore_case);
    xmlFreeTextReader(reader);
    xmlCleanupParser();
  } else {
    fprintf(stderr, "could not open '%s' for reading\n", filename);
  }

  return parse_result;
}

/** Print help and exit. */
static void help(char *name) {
  fprintf(
      stderr,
      "usage: %s [-hi] [-e encoding] [-p prefix] name [infiles]\n\n",
      basename(name));
  fputs("extract content for a particular element (name) from XML\n\n", stderr);
  fputs("-h      print this help and exit\n", stderr);
  fputs("-i      ignore case of name (and prefix)\n", stderr);
  fputs("-e ENC  set encoding (default: UTF-8)\n", stderr);
  fputs("-p PFX  match prefix, too\n", stderr);
  exit(0);
}

/** Execute an extraction process. */
int main(int argc, char **argv) {
  int show_help = 0;
  int ignore_case = 0;
  int exit_status = EXIT_FAILURE;
  int c;
  char *encoding = "UTF-8";
  char *prefix = NULL;
  char *name = NULL;

  // option parsing
  while ((c = getopt(argc, argv, "hie:p:")) != -1)
    switch (c)
    {
      case 'h': show_help = 1; break;
      case 'i': ignore_case = 1; break;
      case 'e': encoding = optarg; break;
      case 'p': prefix = optarg; break;
      case '?': break; // getopt prints an error message
      default: abort();
    }

  // print help and exit if requested
  if (show_help) help(argv[0]);

  // ensure name argument
  if (argc - optind < 1) {
    fputs("name argument missing\n", stderr);
    exit(EXIT_FAILURE);
  } else {
    name = argv[optind++];
  }

  if (ignore_case) {
    strtoupper(name);
    if (prefix != NULL) strtoupper(prefix);
  }

  if (optind < argc) {
    // parse input files
    for (; optind < argc; optind++) {
      exit_status = parseFile(argv[optind], encoding, prefix, name, ignore_case);

      if (exit_status != EXIT_SUCCESS) {
        fprintf(stderr, "failed to parse '%s'\n", argv[optind]);
        break;
      }
    }

    if (optind == argc) exit_status = EXIT_SUCCESS;
  } else {
    // streaming XML from STDIN
    xmlTextReaderPtr reader = xmlReaderForFd(fileno(stdin), NULL, encoding, 0);

    if (reader != NULL) {
      if ((exit_status = parse(reader, prefix, name, ignore_case)) != EXIT_SUCCESS)
        fputs("failed to parse the standard input stream\n", stderr);

      xmlFreeTextReader(reader);
    } else {
      fputs("failed to read from standard input\n", stderr);
    }
  }

  return exit_status;
}
