/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: teng.h,v 1.3 2007-05-21 15:43:28 vasek Exp $
 *
 * DESCRIPTION
 * Teng engine.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 */


#ifndef TENG_H
#define TENG_H

#include <string>
#include <vector>
#include <utility>

#include <tengstructs.h>
#include <tengwriter.h>
#include <tengerror.h>
#include <tengconfig.h>

namespace Teng {

class TemplateCache_t;
class FilesystemInterface_t;

/** @short Templating engine.
 */
class Teng_t {
public:
    /** @short Templating engine.
     */
    struct Settings_t {
        inline Settings_t(unsigned int programCacheSize = 0,
                          unsigned int dictCacheSize = 0)
            : programCacheSize(programCacheSize),
              dictCacheSize(dictCacheSize)
        {
            // no-op
        }

        inline Settings_t(int, bool,
                          unsigned int programCacheSize = 0,
                          unsigned int dictCacheSize = 0)
            : programCacheSize(programCacheSize),
              dictCacheSize(dictCacheSize)
        {
            // no-op
        }

        unsigned int programCacheSize;
        unsigned int dictCacheSize;
    };

    /** @short Create new engine.
     *  @param root root of relative paths
     *  @param settings teng options
     */
    Teng_t(const std::string &root, const Settings_t &settings);

    /** @short Create new engine.
     *  @param settings teng options
     *  @param filesystem Filesystem to use; ownership is transferred
     */
    Teng_t(FilesystemInterface_t *filesystem, const Settings_t &settings);

    /** @short Destroy engine.
     */
    ~Teng_t();

    /** @short Generate page from file template.
     *  @param templateFilename file with main template
     *  @param skin skin of template
     *  @param dict language dictionary
     *  @param lang language
     *  @param param config (dictionary with non language data)
     *  @param contentType content type of page
     *  @param encoding encoding of page
     *  @param data data tree
     *  @param writer output writer (page destinatin)
     *  @param err error log
     *  @return 0 OK, !0 error
     */
    int generatePage(const std::string &templateFilename,
                     const std::string &skin,
                     const std::string &dict, const std::string &lang,
                     const std::string &param, const std::string &contentType,
                     const std::string &encoding, const Fragment_t &data,
                     Writer_t &writer, Error_t &err);

    /** @short Generate page from string template.
     *  @param templateString main template
     *  @param dict language dictionary
     *  @param lang language
     *  @param param config (dictionary with non language data)
     *  @param contentType content type of page
     *  @param encoding encoding of page
     *  @param data data tree
     *  @param writer output writer (page destinatin)
     *  @param err error log
     *  @return 0 OK, !0 error
     */
    int generatePage(const std::string &templateString,
                     const std::string &dict, const std::string &lang,
                     const std::string &param, const std::string &contentType,
                     const std::string &encoding, const Fragment_t &data,
                     Writer_t &writer, Error_t &err);

    /** @short Find entry in dictionary.
     *  @param config config dictionary path
     *  @param dict language dictionary path
     *  @param lang language
     *  @param key name of entry
     *  @param value found entry
     *  @return 0 entry found, !0 entry not found
     */
    int dictionaryLookup(const std::string &config, const std::string &dict,
                         const std::string &lang, const std::string &key,
                         std::string &value);

    /**
     * @short Lists supported content types.
     * @param supported list of supported content types.
     */
    static void listSupportedContentTypes(std::vector<std::pair<std::string,
                                          std::string> > &supported);


    enum {
        /** @short Appends error log in the output when set.
         */
        LM_LOG_TO_OUTPUT         = 0x0001,

        /** @short Enables acces to error fragment from templete.
         *  When not set error fragment seems to have always no element.
         */
        LM_ERROR_FRAGMENT        = 0x0002,
    };

private:
    /** @short Copy constructor intentionaly private -- copying
     *         disabled.
     */
    Teng_t(const Teng_t&);

    /** @short Assignment operator intentionally private -- assignment
     *         disabled.
     */
    Teng_t operator=(const Teng_t&);

    void init(const Settings_t &settings);

    /** @short Holds filesystem implementation.
     */
    FilesystemInterface_t *filesystem;

    /** @short Cache of templates.
     */
    TemplateCache_t *templateCache;

    /** @short Error log.
     */
    Error_t err;
};

} // namespace Teng

#endif // TENG_H

