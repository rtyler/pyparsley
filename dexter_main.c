#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kstring.h"
#include "printbuf.h"
#include "dexter.h"
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlwriter.h>
#include <json/json.h>

struct json_object * recurse(xmlNodePtr);

void dex_error(char* msg){
	fprintf(stderr, "%s\n", msg);
  exit(1);
}

int main (int argc, char **argv) {
  if(argc != 3) {
    printf("Usage: dexter DEX HTML\n");
    exit(1);
  }
  struct printbuf *dex = printbuf_new();
  
  FILE * fd = fopen(argv[1], "r");
  
  printbuf_file_read(fd, dex);
  
  xmlParserCtxtPtr ctxt = xmlNewParserCtxt();
  char* xslt = dex_compile(dex->buf, "");
  xmlDocPtr doc = xmlCtxtReadMemory(ctxt, xslt, strlen(xslt), "http://foo", "UTF-8", 3);
  xsltStylesheetPtr stylesheet = xsltParseStylesheetDoc(doc);
  
  htmlParserCtxtPtr htmlCtxt = htmlNewParserCtxt();
  htmlDocPtr html = htmlCtxtReadFile(htmlCtxt, argv[2], "UTF-8", 3);
  
  xmlDocPtr xml = xsltApplyStylesheet(stylesheet, html, NULL);
  
  // walk xml
  
  struct json_object *json = recurse(xml->children->children);
  
  printf("json: %s\n", json_object_to_json_string(json));
  
	return 0;
}

struct json_object * recurse(xmlNodePtr xml) {
  if(xml == NULL) return NULL;
  
  xmlNodePtr child;
  struct json_object * json = json_object_new_string("bogus");
  
  switch(xml->type) {
    case XML_ELEMENT_NODE:
      child = xml->children;
      if(xml->ns == NULL) {
        child = xml;
        json = json_object_new_object();
        while(child != NULL) {
          json_object_object_add(json, child->name, recurse(child->children));
          child = child->next;
        }
      } else if(!strcmp(xml->ns->prefix, "dexter")) {
        if(!strcmp(xml->name, "groups")) {
          json = json_object_new_array();          
          while(child != NULL) {
            json_object_array_add(json, recurse(child->children));
            child = child->next;
          }          
        } else if(!strcmp(xml->name, "group")) {
          // Implicitly handled by dexter:groups handler
        }
      }
      break;
    case XML_TEXT_NODE:
      json = json_object_new_string(xml->content);
      break;
  }
  return json;
}