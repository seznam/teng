#!/usr/bin/python -S
# -*- coding: utf-8 -*-
#
# Teng -- a general purpose templating engine.
# Copyright (C) 2004  Seznam.cz, a.s.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Seznam.cz, a.s.
# Naskove 1, Praha 5, 15000, Czech Republic
# http://www.seznam.cz, mailto:teng@firma.seznam.cz
#
#
# $Id: generate.py,v 1.2 2006-07-12 17:03:59 sten__ Exp $
#
# DESCRIPTION
# Python generatePage wrapper
#
# AUTHORS
# Jan Adamek (jan.adamek@firma.seznam.cz)
#
# HISTORY
# 2006-07-11  (sten__)
#             Created.
#
# USAGE
# ./generate.py [-conf configuration] [-content contenttype] [-dict dictionary]
#             [-exp expected] [-help][-lang language] [-out output] [-quiet]
#             [-templ template] [-xml data]
#

import sys

if __name__ != '__main__':
    print sys.argv[0], 'cannot be used as module'
    sys.exit(1)

if hasattr(sys, 'setdefaultencoding'):
    sys.setdefaultencoding('utf-8')
    del sys.setdefaultencoding
else:
    if '-quiet' not in sys.argv[1:]:
        sys.stderr.write('%s: FUTURE ERROR WARNING: Unable to set encoding. This can cause error when printing non-ASCII characters\n' % sys.argv[0])
        sys.stderr.flush()
import os
import site
import locale
try:
    import teng
except Exception, e:
    if '-quiet' not in sys.argv[1:]:
        sys.stderr.write('%s: Unable to load Teng module: %s\n' % (sys.argv[0], e))
        sys.stderr.flush()
    sys.exit(1)
import xml.dom.minidom as xml

error = False
verbose = True
source = {
    'conf' : None,
    'content' : 'text/html',
    'dict' : None,
    'lang' : locale.getdefaultlocale()[0]
}
source['lang'] = source['lang'][:source['lang'].find('_')]
args = sys.argv[1:]
while len(args):
    if args[0] == '-help':
        if '-quiet' not in sys.argv[1:]:
            print 'Usage: %s [-conf configuration] [-content contenttype]' % sys.argv[0]
            print '\t\t[-dict dictionary] [-exp expected] [-help] [-lang language]'
            print '\t\t[-out output] [-quiet] [-templ template] [-xml data]'
            print '\t-conf configuration'
            print '\t\tConfiguration (language independent) dictionary filename'
            print '\t-content contenttype'
            print '\t\tContent type (defaults to ’text/html‘)'
            print '\t-dict dictionary'
            print '\t\tLanguage dependent dictionary filename'
            print '\t-exp expected'
            print '\t\tExpected output filename; switches to test mode'
            print '\t-lang language'
            print '\t\tLanguage (defaults to language defined by locales)'
            print '\t-out output'
            print '\t\tOutput filename; if none specified, stdandard output is used'
            print '\t-quiet'
            print '\t\tQuiet mode; do not print anything except for generation output'
            print '\t-temp template'
            print '\t\tTemplate filename; if none specified, standard input is used'
            print '\t-xml data'
            print '\t\tData XML filename'
        sys.exit(0)
    if args[0] == '-quiet':
        verbose = False
        args = args[1:]
    elif args[0][1:] in ('conf', 'dict', 'exp', 'lang', 'out', 'templ', 'xml'):
        if len(args) == 1:
            if verbose:
                sys.stderr.write('%s: parameter ’%s‘ requires an argument\n' % (sys.argv[0], args[0][1:]))
            error = True
            args = {}
        else:
            source[args[0][1:]] = args[1]
            args = args[2:]
    else:
        if verbose:
            if args[0][0] == '-':
                sys.stderr.write('%s: unknown parameter ’%s‘\n' % (sys.argv[0], args[0][1:]))
            else:
                sys.stderr.write('%s: expected parameter but ’%s‘ found\n' % (sys.argv[0], args[0]))
        args = args[1:]
        error = True

for name in ('conf', 'templ', 'xml'):
    if name in source.keys() and source[name] is not None:
        if not os.path.isfile(source[name]):
            if verbose:
                sys.stderr.write('%s: invalid file ’%s‘ specified for parameter ’%s‘\n' % (sys.argv[0], source[name], file))
            error = True

if error:
    sys.exit(1)

data = {}
errors = {}
if 'xml' in source.keys():
    try:
        dom = xml.parse(source['xml'])
    except Exception, e:
        if verbose:
            sys.stderr.write('%s: Exception raised when parsing ’%s‘: ’%s‘\n' % (sys.argv[0], source['xml'], e))
            sys.stderr.flush()
        sys.exit(2)
    
    class ParseException(Exception):
        def __init__(self, message):
            Exception.__init__(self, message, [])
    
        def __str__(self):
            args = self.args[1]
            args.reverse()
            return self.args[0] % '/'.join(args)
    
        def append(self, name):
            self.args[1].append(name)
            return self
    
    class DuplicateException(ParseException):
        pass
    
    def getText(nodes):
        text = ''
        for node in nodes:
            if node.nodeType in (node.TEXT_NODE, node.CDATA_SECTION_NODE):
                text += node.data
        return text
    
    def parseDict(nodes, root = False):
        dict = {
            None : {}
        }
        for node in nodes:
            if node.nodeType == node.ELEMENT_NODE:
                name = '{%s}' % node.tagName
                try:
                    if node.getAttribute('name'): # Each dict element must have name
                        name = node.getAttribute('name').encode('utf-8')
                        if name in dict:
                            raise DuplicateException('Dictionary ’%s‘ already has an attribute called ’%s‘' % ('%s', name))
                        if node.tagName == 'binary':
                            if node.getAttribute('file'):
                                if os.path.isfile(node.getAttribute('file')):
                                    handle = file(node.getAttribute('file'), 'rb')
                                    dict[name] = handle.read()
                                    handle.close()
                                else:
                                    raise ParseException('File does not exist or is not regular in ’%s‘').append(name)
                            else:
                                raise ParseException('Tag ’binary‘ lacks required element file in ’%s‘').append(name)
                        elif node.tagName == 'dict':
                            dict[name] = parseDict(node.childNodes)
                        elif node.tagName == 'float':
                            try:
                                dict[name] = float(getText(node.childNodes))
                            except Exception, e:
                                raise ParseException('’%s‘ in ’%s‘' % (str(e), '%s')).append(name)
                        elif node.tagName == 'frags':
                            dict[name] = parseFrags(node.childNodes)
                        elif node.tagName == 'number':
                            try:
                                dict[name] = int(getText(node.childNodes))
                            except Exception, e:
                                raise ParseException('’%s‘ in ’%s‘' % (str(e), '%s')).append(name)
                        elif node.tagName == 'string':
                            dict[name] = getText(node.childNodes)
                        else:
                            raise ParseException('Invalid tag ’%s‘ in ’%s‘' % (node.tagName, '%s'))
                    elif node.getAttribute('file'):
                        if node.tagName == 'errors' and root:
                            if node.getAttribute('file') in dict[None].keys():
                                raise DuplicateError('There was already error dictionary for file ’%s‘ specified in dictionary ’%s‘' % node.getAttribute('file'))
                            dict[None][node.getAttribute('file')] = parseErrors(node.childNodes)
                        else:
                            raise ParseException('Invalid tag ’%s‘ in ’%s‘' % (node.tagName, '%s'))
                    else:
                            raise ParseException('Tag ’%s‘ is lacks required argument in ’%s‘' % (node.tagName, '%s'))
                except ParseException, e:
                    raise e.append(name)
        return dict
    
    def parseErrors(nodes):
        errors = {}
        for node in nodes:
            if node.nodeType == node.ELEMENT_NODE:
                if node.getAttribute('line') and node.getAttribute('column'):
                    if node.tagName == 'error':
                        try:
                            errors['%d:%d' % (int(node.getAttribute('line')), int(node.getAttribute('column')))] = False
                        except Exception, e:
                            raise ParseException('’%s‘ in ’%s‘' % (str(e), '%s'))
                    else:
                        raise ParseException('Invalid tag ’%s‘ in ’%s‘' % (node.tagName, '%s'))
                else:
                    raise ParseException('Tag ’%s‘ lacks one or more required attributes in ’%s‘' % (node.tagName, '%s'))
        return errors
    
    def parseFrags(nodes):
        frags = []
        for node in nodes:
            if node.nodeType == node.ELEMENT_NODE:
                if node.tagName == 'frag':
                    frags.append(parseDict(node.childNodes))
                else:
                    raise ParseException('Invalid tag ’%s‘ in ’%s‘' % (node.tagName, '%s'))
        return frags
    
    error = True
    for node in dom.childNodes:
        if node.nodeType == node.ELEMENT_NODE and node.tagName == 'tengData':
            error = False
            try:
                data = parseDict(node.childNodes, True)
            except Exception, e:
                if verbose:
                    sys.stderr.write('%s: %s\n' % (sys.argv[0], e))
                    sys.stderr.flush()
                sys.exit(4)
            break
    
    dom.unlink()
    del dom

    if error:
        if verbose:
            sys.stderr.write('%s: No ’tengData‘ root element found in file ’%s‘\n' % (sys.argv[0], sys.argv[2]))
            sys.stderr.flush()
        sys.exit(3)

    errors = data[None]
    expected = 0
    for err in errors.values():
        expected += len(err)
    del data[None]

# endif 'xml' in source.keys()

engine = teng.Teng(contentType = source['content'], encoding = 'utf-8', errorFragment = 1)
if source['dict'] is None:
    source['lang'] = None
if 'templ' in source.keys():
    page = engine.generatePage(source['templ'], data = data, language = source['lang'], dictionaryFilename = source['dict'], configFilename = source['conf'])
else:
    page = engine.generatePage(templateString = sys.stdin.read(), data = data, language = source['lang'], dictionaryFilename = source['dict'], configFilename = source['conf'])
status = page['status']
if 'out' in source.keys():
    try:
        output = file(source['out'], 'wb')
    except:
        if verbose:
            sys.stderr.write('%s: Failed writing to output file ’%s‘' % (sys.argv[0], fource['out']))
        sys.exit(1)
else:
    output = sys.stdout
if 'exp' not in source.keys():
    output.write(page['output'])
    output.flush()
    sys.exit(-status)
error = False
exduplicates = 0
matched = 0
unerrors = {}
unexpected = 0
unduplicates = 0
if status:
    if not page['output']:
        output.write('Generation terminated with status %d due to following errors:\n' % page['status'])
    else:
        output.write('Generation finished with status %d with following errors:\n' % page['status'])
    output.write('\n')
    output.flush()
    for err in page['errorLog']:
        position = '%d:%d' % (err['line'], err['column'])
        src = os.path.basename(err['filename'])
        if src in errors.keys() and position in errors[src].keys():
            if errors[src][position]:
                exduplicates += 1
                output.write('Expected error (DUP): %s\n' % err['message'])
            else:
                matched += 1
                errors[src][position] = True
                output.write('Expected error: %s\n' % err['message'])
        else:
            error = True
            if not src in unerrors.keys():
                unerrors[src] = []
            if position in unerrors[src]:
                unduplicates += 1
                output.write('Unexpected error (DUP): %s\n' % err['message'])
            else:
                unerrors[src].append(position)
                unexpected += 1
                output.write('Unexpected error: %s\n' % err['message'])
        output.write('\tLevel: %d\n' % err['level'])
        output.write('\tFile: %s\n' % err['filename'])
        output.write('\tPosition: %s\n' % position)
    if not page['output']:
        sys.exit(5)
    output.write('\n')
    output.flush()
    if expected:
        if expected == matched:
            output.write('All expected errors were generated\n')
        else:
            error = True
            output.write('There are some expected errors that were not generated\n')
    else:
        output.write('No errors were expected\n')
    output.write('\nTotal errors:\n')
    output.write('\tReported: %d\n' % len(page['errorLog']))
    output.write('\t\tUnique: %d\n' % (matched + unexpected))
    output.write('\t\tDuplicates: %d\n' % (exduplicates + unduplicates))
    output.write('\tExpected: %d\n' % expected)
    output.write('\t\tMatched: %d\n' % matched)
    output.write('\t\tDuplicates: %d\n' % exduplicates)
    output.write('\tUnexpected: %d\n' % unexpected)
    output.write('\t\tDuplicates: %d\n' % unduplicates)
else:
    output.write('Generation successfully finished with no errors\n')
output.write('\n')
output.flush()

expected = file(source['exp'], 'rb').readlines()
pagelen = len(page['output'])
page = page['output']
strip = 0
if 'templ' in source:
    size = os.path.getsize(source['exp'])
else:
    size = len(template)

if pagelen == size:
    output.write('Generated output has the same size as expected one (%d bytes)\n' % pagelen)
else:
    output.write('Generated output has different size than expected one (%d and %d bytes)\n' % (pagelen, size))
output.write('\n')
output.flush()

if page[len(page) - 1] == '\n': # Strip newline at the end of the document because file.readlines() does the same
    if page[len(page) - 2] == '\r':
        strip = 2
    else:
        strip = 1
elif page[len(page) - 1] == '\r':
    strip = 1

page = page[:len(page) - strip].replace('\r\n', '\n').replace('\r', '\n').split('\n')

i = 0
lines = len(expected)
for line in page:
    if not len(expected):
        output.write('Run out of expected output\'s lines while generated one still have %d more to go\n' % (len(page) - i))
        error = True
        break
    i += 1
    gline = line.decode('utf-8')
    eline = expected[0].decode('utf-8').strip('\r\n')
    if gline != eline:
        error = True
        output.write('Line mismatch at line %d:\n' % i)
        output.write('\t- %s\n' % eline)
        output.write('\t+ %s\n' % gline)
    del expected[0]

if len(expected):
    output.write('Run out of generated output\'s lines while expected one still have %d more to go\n' % len(expected))
    error = True

if error:
    output.write('\n')
    output.flush()
output.write('Total lines:\n')
output.write('\tChecked: %d\n' % i)
output.write('\tExpected: %d\n' % lines)
output.write('\tGenerated: %d\n' % len(page))

if error:
    output.write('Generated output and/or errors do not match expected ones\n')
    output.flush()
    sys.exit(6)
else:
    output.write('Generated output and errors exactly match expected ones\n')
    output.flush()
    sys.exit(-status)
