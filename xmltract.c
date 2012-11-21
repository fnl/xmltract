#include <libgen.h> // basename()
#include <libxml/xmlreader.h>
#include <libxml/tree.h>

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

/** Recurse the document tree, printing the matching element names' content. */
void printContent(xmlNode *root, const char *name) {
  xmlNode * node;

  // iterate over this node
  for (node = root; node; node = node->next) {
    if (node->type == XML_ELEMENT_NODE) {
      const char * n = (const char *) node->name;

      // extract node content for matching names
      if (strcmp(name, n) == 0) {
        xmlChar *content = xmlNodeGetContent(node);

        if (content != NULL) {
          size_t len = strlen((char *) content);
          char buffer[len];

          // print non-empty, timmed content
          if (trim(&buffer[0], len, (char *) content) > 0) puts(buffer);
          xmlFree(content);
        }
      }

      // recurse over children
      printContent(node->children, name);
    }
  }
}

/** Extract the contents of a particular XML element (name) from a file input stream. */
int parse(xmlTextReaderPtr reader, const char *name) {
  int result;
  xmlDocPtr doc;

  // parse a document, preserving matching name nodes
  if ((result = xmlTextReaderPreservePattern(reader, (const xmlChar *) name, NULL)) >= 0) {
    while ((result = xmlTextReaderRead(reader) == 1)) continue;

    // create a documten from the preserved nodes and print their content
    if (result == 0) {
      doc = xmlTextReaderCurrentDoc(reader);
      printContent(xmlDocGetRootElement(doc), name);
      xmlFreeDoc(doc);
    }
  }

  return result;
}

/** Extract the particular XML content from a file. */
int parseFile(const char *filename, const char *encoding, const char *name) {
  xmlTextReaderPtr reader = xmlReaderForFile(filename, encoding, 0);
  int parse_result = -1;

  // parse the file if the reader could be created
  if (reader) {
    g_message("parsing '%s'", filename);
    parse_result = parse(reader, name);
    xmlFreeTextReader(reader);
    xmlCleanupParser();
  } else {
    g_error("could not open '%s' for reading", filename);
  }

  return parse_result;
}

/** Print help and exit. */
static void help(char *name) {
  fprintf(stderr, "usage: %s [-hqv] [-e encoding] name [infiles]\n\n", basename(name));
  fputs("extract content for a particular element (name) from XML\n\n", stderr);
  fputs("-e ENC  set encoding (default: UTF-8)\n", stderr);
  fputs("-h      print this help and exit\n", stderr);
  fputs("-q      quiet logging (errors only)\n", stderr);
  fputs("-v      verbose logging (default: warnings)\n", stderr);
  exit(0);
}

/** Default log handler does nothing. */
static void silent_handler(
    const gchar *log_domain,
    GLogLevelFlags log_level,
    const gchar *message,
    gpointer user_data) {
  return;
}

/** This log handler prints to STDERR. */
static void stderr_handler(
    const gchar *log_domain,
    GLogLevelFlags log_level,
    const gchar *message,
    gpointer user_data) {
  char buffer[20];
  time_t now = time(NULL);
  struct tm *now_p = localtime(&now);
  char *time_p = &buffer[0];
  char *level_p;

  switch (log_level & G_LOG_LEVEL_MASK)
  {
    case G_LOG_LEVEL_ERROR:    level_p = "ERRO"; break;
    case G_LOG_LEVEL_CRITICAL: level_p = "CRIT"; break;
    case G_LOG_LEVEL_WARNING:  level_p = "WARN"; break;
    case G_LOG_LEVEL_MESSAGE:  level_p = "MESG"; break;
    case G_LOG_LEVEL_INFO:     level_p = "INFO"; break;
    case G_LOG_LEVEL_DEBUG:    level_p = "DEBG"; break;
    default:                   level_p = "LVL?";
  }

  if (strftime(time_p, 20, "%Y-%M-%d %H:%M:%S", now_p) == 0)
    time_p = asctime(now_p);

  fprintf(stderr, "%s %s: %s: %s\n", level_p, time_p, log_domain, message);
}

int main(int argc, char **argv) {
  int verbosity = 1;
  int show_help = 0;
  int name_idx = 1;
  int exit_status = EXIT_FAILURE;
  int c;
  char *encoding = "UTF-8";

  // default logging: silence
  g_log_set_default_handler(silent_handler, NULL);

  // option parsing
  while ((c = getopt(argc, argv, "hqve:")) != -1)
    switch (c)
    {
      case 'h': show_help = 1; break;
      case 'v': if (verbosity == 1) verbosity = 2; break;
      case 'q': if (verbosity == 1) verbosity = 0; break;
      case 'e': encoding = optarg; break;
      case '?': break; // getopt prints an error message
      default: abort();
    }

  // logging setup
  GLogLevelFlags log_level = G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_ERROR | G_LOG_FLAG_FATAL;
  if (verbosity > 0) log_level |= G_LOG_LEVEL_WARNING;
  if (verbosity > 1) log_level |= G_LOG_LEVEL_MESSAGE;
  if (verbosity > 2) log_level |= G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG;
  g_log_set_handler(G_LOG_DOMAIN, log_level, stderr_handler, NULL);

  // print help and exit if requested
  if (show_help) help (argv[0]);

  // ensure name argument
  if (argc - optind < 1) {
    fputs("name argument missing\n", stderr);
    exit(EXIT_FAILURE);
  } else {
    name_idx = optind++;
  }

  if (optind < argc) {
    // parse input files
    for (; optind < argc; optind++)
      if ((c = parseFile(argv[optind], encoding, argv[name_idx])) != 0)
        g_error("XML reader failed to parse '%s'", argv[optind]);

    if (optind == argc && c == 0) exit_status = EXIT_SUCCESS;
  } else {
    // streaming XML from STDIN
    g_message("%s streaming mode", encoding);
    xmlTextReaderPtr reader = xmlReaderForFd(fileno(stdin), NULL, encoding, 0);
    if (parse(reader, argv[name_idx]) == 0) exit_status = EXIT_SUCCESS;
    else g_critical("XML reader failed to parse the input stream");
    xmlFreeTextReader(reader);
  }

  return exit_status;
}
