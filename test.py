#!/usr/bin/python -S
# -*- coding: utf-8 -*-

import sys

if __name__ != '__main__':
    print __file__, 'cannot be used as module'
    sys.exit(1)

if hasattr(sys, 'setdefaultencoding'):
    sys.setdefaultencoding('utf-8')
    del sys.setdefaultencoding
else:
    sys.stderr.write('FUTURE ERROR WARNING: Unable to set encoding. This can cause error when printing non-ASCII characters\n')
    sys.stderr.flush()
import site
import os
try:
    import teng
except:
    print 'Unable to load Teng module'
    sys.exit(1)
import xml.dom.minidom as xml

if len(sys.argv) != 4:
    print 'Usage:', __file__, 'template data expected_output'
    sys.exit(0)

error = False
if not os.path.isfile(sys.argv[1]):
    print 'Invalid template specified: not exists or not a file'
    error = True

if not os.path.isfile(sys.argv[2]):
    print 'Invalid data specified: not exists or not a file'
    error = True

if not os.path.isfile(sys.argv[3]):
    print 'Invalid expected_output specified: not exists or not a file'
    error = True

if error:
    sys.exit(1)

try:
    dom = xml.parse(sys.argv[2])
except Exception, e:
    print 'Exception raised when parsing "%s":' % sys.argv[2], e
    sys.exit(2)

class ParseException(Exception):
    def __init__(self, message):
        Exception.__init__(self, message, [])

    def __str__(self):
        return self.args[0] % '/'.join(self.args[1])

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
                        raise DuplicateException('Dictionary %s already has an attribute called "%s"' % ('%s', name))
                    if node.tagName == 'binary':
                        if node.getAttribute('file'):
                            if os.path.isfile(node.getAttribute('file')):
                                handle = file(node.getAttribute('file'), 'rb')
                                dict[name] = handle.read()
                                handle.close()
                            else:
                                raise ParseException('File does not exist or is not regular in %s').append(name)
                        else:
                            raise ParseException('Tag binary lacks required element file in %s').append(name)
                    elif node.tagName == 'dict':
                        dict[name] = parseDict(node.childNodes)
                    elif node.tagName == 'float':
                        try:
                            dict[name] = float(getText(node.childNodes))
                        except Exception, e:
                            raise ParseException('%s in %s' % (str(e), '%s')).append(name)
                    elif node.tagName == 'frags':
                        dict[name] = parseFrags(node.childNodes)
                    elif node.tagName == 'number':
                        try:
                            dict[name] = int(getText(node.childNodes))
                        except Exception, e:
                            raise ParseException('%s in %s' % (str(e), '%s')).append(name)
                    elif node.tagName == 'string':
                        dict[name] = getText(node.childNodes)
                    else:
                        raise ParseException('Invalid tag "%s" in %s' % (node.tagName, '%s'))
                elif node.getAttribute('file'):
                    if node.tagName == 'errors' and root:
                        if node.getAttribute('file') in dict[None].keys():
                            raise DuplicateError('There was already error dictionary for file %s specified in dictionary %s' % node.getAttribute('file'))
                        dict[None][node.getAttribute('file')] = parseErrors(node.childNodes)
                    else:
                        raise ParseException('Invalid tag "%s" in %s' % (node.tagName, '%s'))
                else:
                        raise ParseException('Tag "%s" is lacks required argument in %s' % (node.tagName, '%s'))
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
                        raise ParseException('%s in %s' % (str(e), '%s'))
                else:
                    raise ParseException('Invalid tag "%s" in %s' % (node.tagName, '%s'))
            else:
                raise ParseException('Tag "%s" lacks one or more required attributes in %s' % (node.tagName, '%s'))
    return errors

def parseFrags(nodes):
    frags = []
    for node in nodes:
        if node.nodeType == node.ELEMENT_NODE:
            if node.tagName == 'frag':
                frags.append(parseDict(node.childNodes))
            else:
                raise ParseException('Invalid tag "%s" in %s' % (node.tagName, '%s'))
    return frags

error = True
for node in dom.childNodes:
    if node.nodeType == node.ELEMENT_NODE and node.tagName == 'tengData':
        error = False
        try:
            data = parseDict(node.childNodes, True)
            attributes = {
                'conf' : node.getAttribute('conf'),
                'def' : node.getAttribute('def'),
                'dict' : node.getAttribute('dict'),
                'lang' : node.getAttribute('lang')
            }
        except Exception, e:
            print e
            sys.exit(4)
        break

dom.unlink()
del dom

errors = data[None]
expected = 0
for err in errors.values():
    expected += len(err)
del data[None]

if error:
    print 'No "tengData" root element found in file "%s"' % sys.argv[2]
    sys.exit(3)

engine = teng.Teng(contentType = 'text/html', encoding = 'utf-8', errorFragment = 1, validate = 1)
page = engine.generatePage(sys.argv[1], data = data, language = attributes['lang'], dictionaryFilename = attributes['dict'], configFilename = attributes['conf'], dataDefinitionFilename = attributes['def'])
status = page['status']
error = False
exduplicates = 0
matched = 0
unerrors = {}
unexpected = 0
unduplicates = 0
if status:
    if not page['output']:
        print 'Generation terminated with status %d due to following errors:', page['status']
    else:
        print 'Generation finished with status %d with following errors:' % page['status']
    print
    for err in page['errorLog']:
        position = '%d:%d' % (err['line'], err['column'])
        source = os.path.basename(err['filename'])
        if source in errors.keys() and position in errors[source].keys():
            if errors[source][position]:
                exduplicates += 1
                print 'Expected error (DUP):', err['message']
            else:
                matched += 1
                errors[source][position] = True
                print 'Expected error:', err['message']
        else:
            error = True
            if not source in unerrors:
                unerrors[source] = []
            if position in unerrors[source]:
                unduplicates += 1
                print 'Unexpected error (DUP):', err['message']
            else:
                unerrors[source].append(position)
                unexpected += 1
                print 'Unexpected error:', err['message']
        print '\tLevel:', err['level']
        print '\tFile:', err['filename']
        print '\tPosition: %s' % position
    if not page['output']:
        sys.exit(5)
    print
    if expected:
        if expected == matched:
            print 'All expected errors were generated'
        else:
            error = True
            print 'There are some expected errors that were not generated'
    else:
        print 'No errors were expected'
    print
    print 'Total errors:'
    print '\tReported: %d' % len(page['errorLog'])
    print '\t\tUnique: %d' % (expected + unexpected)
    print '\t\tDuplicates: %d' % (exduplicates + unduplicates)
    print '\tExpected: %d' % expected
    print '\tMatched: %d' % matched
    print '\t\tDuplicates: %d' % exduplicates
    print '\tUnexpected: %d' % unexpected
    print '\t\tDuplicates: %d' % unduplicates
else:
    print 'Generation successfully finished with no errors'
print

expected = file(sys.argv[3], 'rb').readlines()
pagelen = len(page['output'])
page = page['output']
strip = 0

if len(page) == os.path.getsize(sys.argv[3]):
    print 'Generated output has the same size as expected one (%d bytes)' % len(page)
else:
    print 'Generated output has different size than expected one (%d and %d bytes)' % (len(page), os.path.getsize(sys.argv[3]))
print

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
        print 'Run out of expected output\'s lines while generated one still have %d more to go' % (len(page) - i)
        error = True
        break
    i += 1
    gline = line.decode('utf-8')
    eline = expected[0].decode('utf-8').strip('\r\n')
    if gline != eline:
        error = True
        print 'Line mismatch at line %d:' % i
        print '\t- %s' % eline
        print '\t+ %s' % gline
    del expected[0]

if len(expected):
    print 'Run out of generated output\'s lines while expected one still have %d more to go' % len(expected)
    error = True

if error:
    print
print 'Total lines:'
print '\tChecked:', i
print '\tExpected:', lines
print '\tGenerated:', len(page)

if error:
    print 'Generated output and/or errors do not match expected ones'
    sys.exit(6)
else:
    print 'Generated output and errors exactly match expected ones'
    sys.exit(-status)
