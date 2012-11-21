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

  return result;
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
    g_message("parsing '%s'", filename);
    parse_result = parse(reader, prefix, name, ignore_case);
    xmlFreeTextReader(reader);
    xmlCleanupParser();
  } else {
    g_error("could not open '%s' for reading", filename);
  }

  return parse_result;
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

/** Print help and exit. */
static void help(char *name) {
  fprintf(
      stderr,
      "usage: %s [-hiqv] [-e encoding] [-p prefix] name [infiles]\n\n",
      basename(name));
  fputs("extract content for a particular element (name) from XML\n\n", stderr);
  fputs("-h      print this help and exit\n", stderr);
  fputs("-i      ignore case of name (and prefix)\n", stderr);
  fputs("-q      quiet logging (errors only)\n", stderr);
  fputs("-v      verbose logging (default: warnings)\n", stderr);
  fputs("-e ENC  set encoding (default: UTF-8)\n", stderr);
  fputs("-p PFX  match prefix, too\n", stderr);
  exit(0);
}

int main(int argc, char **argv) {
  int verbosity = 1;
  int show_help = 0;
  int ignore_case = 0;
  int exit_status = EXIT_FAILURE;
  int c;
  char *encoding = "UTF-8";
  char *prefix = NULL;
  char *name = NULL;

  // default logging: silence
  g_log_set_default_handler(silent_handler, NULL);

  // option parsing
  while ((c = getopt(argc, argv, "hiqve:p:")) != -1)
    switch (c)
    {
      case 'h': show_help = 1; break;
      case 'i': ignore_case = 1; break;
      case 'v': if (verbosity == 1) verbosity = 2; break;
      case 'q': if (verbosity == 1) verbosity = 0; break;
      case 'e': encoding = optarg; break;
      case 'p': prefix = optarg; break;
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
    name = argv[optind++];
  }

  if (ignore_case) {
    strtoupper(name);
    if (prefix != NULL) strtoupper(prefix);
    g_message("matching '%s' ignoring case", name);
  } else {
    g_message("matching '%s' case sensitive", name);
  }

  if (optind < argc) {
    // parse input files
    for (; optind < argc; optind++)
      if ((c = parseFile(argv[optind], encoding, prefix, name, ignore_case)) != 0)
        g_error("XML reader failed to parse '%s'", argv[optind]);

    if (optind == argc && c == 0) exit_status = EXIT_SUCCESS;
  } else {
    // streaming XML from STDIN
    g_message("%s streaming mode", encoding);
    xmlTextReaderPtr reader = xmlReaderForFd(fileno(stdin), NULL, encoding, 0);

    if (reader != NULL) {
      if (parse(reader, prefix, name, ignore_case) == 0) exit_status = EXIT_SUCCESS;
      else g_critical("XML reader failed to parse the input stream");
      xmlFreeTextReader(reader);
    }
  }

  return exit_status;
}
