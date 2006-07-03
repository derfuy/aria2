/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#include "Xml2MetalinkProcessor.h"
#include "DlAbortEx.h"
#include "Util.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

Xml2MetalinkProcessor::Xml2MetalinkProcessor():doc(NULL), context(NULL) {}

Xml2MetalinkProcessor::~Xml2MetalinkProcessor() {
  release();
}

void Xml2MetalinkProcessor::release() {
  if(context) {
    xmlXPathFreeContext(context);
    context = NULL;
  }
  if(doc) {
    xmlFreeDoc(doc);
    doc = NULL;
  }
}

Metalinker* Xml2MetalinkProcessor::parseFile(const string& filename) {
  release();
  doc = xmlParseFile(filename.c_str());
  if(doc == NULL) {
    throw new DlAbortEx("Cannot parse metalink file %s", filename.c_str());
  }
  context = xmlXPathNewContext(doc);
  if(context == NULL) {
    throw new DlAbortEx("Cannot create new xpath context");
  }
  string defaultNamespace = "http://www.metalinker.org/";
  if(xmlXPathRegisterNs(context, (xmlChar*)"m",
			(xmlChar*)defaultNamespace.c_str()) != 0) {
    throw new DlAbortEx("Cannot register namespace %s", defaultNamespace.c_str());
  }
  
  string xpath = "/m:metalink/m:files/m:file";
  Metalinker* metalinker = new Metalinker();
  try {
    for(int index = 1; 1; index++) {
      MetalinkEntry* entry = getEntry(xpath+"["+Util::itos(index)+"]");
      if(entry == NULL) {
	break;
      } else {
	metalinker->entries.push_back(entry);
      }
    }
  } catch(Exception* e) {
    delete metalinker;
    throw;
  }
  return metalinker;
}

MetalinkEntry* Xml2MetalinkProcessor::getEntry(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(result == NULL) {
    return NULL;
  }
  xmlXPathFreeObject(result);
  MetalinkEntry* entry = new MetalinkEntry();
  try {
    entry->version = Util::trim(xpathContent(xpath+"/m:version"));
    entry->language = Util::trim(xpathContent(xpath+"/m:language"));
    entry->os = Util::trim(xpathContent(xpath+"/m:os"));
    entry->md5 = Util::trim(xpathContent(xpath+"/m:verification/m:hash[@type=\"md5\"]"));
    entry->sha1 = Util::trim(xpathContent(xpath+"/m:verification/m:hash[@type=\"sha1\"]"));
    for(int index = 1; 1; index++) {
      MetalinkResource* resource =
	getResource(xpath+"/m:resources/m:url["+Util::itos(index)+"]");
      if(resource == NULL) {
	break;
      } else {
	entry->resources.push_back(resource);
      }
    }
  } catch(Exception* e) {
    delete entry;
    throw;
  }

  return entry;
}

MetalinkResource* Xml2MetalinkProcessor::getResource(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(result == NULL) {
    return NULL;
  }
  MetalinkResource* resource = new MetalinkResource();
  try {
    xmlNodeSetPtr nodeSet = result->nodesetval;
    xmlNodePtr node = nodeSet->nodeTab[0];
    string type = Util::trim(xmlAttribute(node, "type"));
    if(type == "ftp") {
      resource->type = MetalinkResource::TYPE_FTP;
    } else if(type == "http") {
      resource->type = MetalinkResource::TYPE_HTTP;
    } else if(type == "bittorrent") {
      resource->type = MetalinkResource::TYPE_BITTORRENT;
    } else {
      resource->type = MetalinkResource::TYPE_NOT_SUPPORTED;
    }
    string pref = Util::trim(xmlAttribute(node, "preference"));
    if(pref.empty()) {
      resource->preference = 100;
    } else {
      resource->preference = STRTOLL(pref.c_str());
    }
    resource->url = Util::trim(xmlContent(node));
  } catch(Exception* e) {
    delete resource;
    throw e;
  }
  return resource;
}

xmlXPathObjectPtr Xml2MetalinkProcessor::xpathEvaluation(const string& xpath) {
  xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*)xpath.c_str(),
						    context);
  if(result == NULL) {
    throw new DlAbortEx("Cannot evaluate xpath %s", xpath.c_str());
  }
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    return NULL;
  }
  return result;
}

string Xml2MetalinkProcessor::xmlAttribute(xmlNodePtr node, const string& attrName) {
  xmlChar* temp = xmlGetNoNsProp(node, (xmlChar*)attrName.c_str());
  if(temp == NULL) {
    return "";
  } else {
    string attr = (char*)temp;
    xmlFree(temp);
    return attr;
  }
}

string Xml2MetalinkProcessor::xmlContent(xmlNodePtr node) {
  xmlChar* temp = xmlNodeGetContent(node);
  if(temp == NULL) {
    return "";
  } else {
    string content = (char*)temp;
    xmlFree(temp);
    return content;
  }
}

string Xml2MetalinkProcessor::xpathContent(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(result == NULL) {
    return "";
  }
  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlNodePtr node = nodeSet->nodeTab[0]->children;
  string content = (char*)node->content;
  xmlXPathFreeObject(result);
  return content;
}
