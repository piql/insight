
# Table of Contents

1.  [Introduction](#orgd37efff)
2.  [About](#org848f6bb)
3.  [System Requirements](#org94a391a)
    1.  [OS-X / Linux](#org58e0b0d)
4.  [Installation](#orgc9b0958)
5.  [Usage](#org134d5ef)
6.  [Sphinx indexer and search engine](#orgec31a91)
7.  [Reports](#orgd6db6a1)
8.  [Journals](#orgfdf4cc7)
    1.  [Creating *searchable-PDFs*](#org2ac13b5)
9.  [Log files](#org96a2449)
10. [Configuration](#org4675467)
    1.  [Tip](#orgfe50afe)
11. [Batch mode](#org7bca128)
    1.  [Batch mode example](#orga24be88)
12. [How to report issues](#orga83e37a)
13. [History](#orgd10e3f4)
    1.  [insight-v1.3.0](#orge1f24fa)
    2.  [insight-v1.2.0](#org029cca5)
    3.  [insight-v1.2.0-beta3](#org9eaf462)
        1.  [New features](#org1c9ba28)
    4.  [innsyn-v1.2.0-beta1](#org0a36f0c)
        1.  [New features](#org0a3e47f)
    5.  [01.07.2020 innsyn-v1.1.0](#orgdd72120)
        1.  [Fixes](#orgacccf30)
    6.  [innsyn-v1.1.0-beta2](#org256ce45)
        1.  [Fixes](#org21fffb0)
    7.  [innsyn-v1.1.0-beta1](#orgddbb8e5)
        1.  [New features](#orge9e019c)
        2.  [Fixes](#org56f71ea)
    8.  [2018.06.01 innsyn-v1.0.0](#org9db0ca6)
        1.  [Fixes](#org48e9dc5)
    9.  [2018.04.13 innsyn-v1.0.0-rc1](#org4651688)
    10. [2018.02.05 innsyn-v1.0.0-beta2](#org95df22d)
    11. [2018.01.18 innsyn-v1.0.0-beta1](#org8614bac)
14. [Development](#org943f71f)
    1.  [pdftotext](#org032d1f7)
        1.  [Windows](#orgc5b69e2)
        2.  [Other](#orgb7d9b38)
    2.  [Ubuntu](#orgfe9eeab)
    3.  [Windows](#orge2319ea)



<a id="orgd37efff"></a>

# Introduction

*Piql Insight* is a fast and flexible archival package / information package (IP) inspection and dissemination tool. 

The application does not validate the IPs apart from validating the existence of attachments. The application is data-driven and a design goal is that is should not contain any references to file formats, XML-tags or keywords. Instead are the views controlled by configuration files and optional regular expressions to transform the XML to user interface friendly keywords. This means that in principle the application can support any XML based IP based format by adding new config files.

For efficient dissemination the application supports batch mode exports with predefined transformations. It also supports creating *searchable-PDFs* where the PDF contains digitized page and embedded OCR metadata.

Insight ships with these formats:

-   NORAK-5: Used by Norwegian legislation to archive government journals.
-   EPJ shipping list: Used when submitting patient journals, either paper or electronic, to an archive.
-   EPJ package list: Used when submitting electronic patient journals to the Norwegian Health Archive.
-   TAR: Insight opens tar files and shows its content. XML files inside the tar can be further examined.


<a id="org848f6bb"></a>

# About

Piql Insight was originally developed for the *Kommunenes Digitale Ressursjsentral (KDRS)* in Norway and released under the name *KDRS Innsyn*. Its job was to support dissemination of archival packages in the *NORAK-5* format, used by the Norwegian legislation. It has later been extended to support multiple IP formats.


<a id="org94a391a"></a>

# System Requirements

Released applications are tested on Windows 10 64bit.


<a id="org58e0b0d"></a>

## OS-X / Linux

The text indexer tool Sphinx must be in the path. 

Install:

    osx$ brew install sphinx
    centos-redhat$ sudo yum install sphinx

Check that tool is available:

    computer$ indexer 
    Sphinx 2.2.11-id64-release (95ae9a6)
    Copyright (c) 2001-2016, Andrew Aksyonoff
    Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)


<a id="orgc9b0958"></a>

# Installation

1.  Extract *insight-v1.3.0.zip*.
2.  Indexing of attachments can require some disk space, consider changing data location
    *REPORTS\_DIR* in *insight.conf* and ensure it points to a location with sufficient disk space.
3.  UI language can be changed by editing *LANGUAGE* in *insight.conf*. Current available languages are *english*, *norwegian-bokmål* and *norwegian-nynorsk*. Default language is *english*. Please note that changing language after launch can cause trouble since some of the paths are localized.


<a id="org134d5ef"></a>

# Usage

![img](./screenshot2.png)

1.  Launch *insight.exe* (OS-X/Linux: insight) from *innsyn-v1.3.0*.
2.  The UI has four main elements:
    1.  **Node tree**: Shows all the elements in the imported XML/package. Each node in the tree corresponds to an XML tag or a file. It also shows previously loaded packages. These can be reloaded or deleted by using the **Import** or **Delete** buttons in the node-info window.
    2.  **Node info**: Shows detailed info for each node. Some elements have action buttons attached to them. For example has the element for attachments a **Display** button that allows the user to display the attachment with the viewer the operating system has associated with this file format.
    3.  **Search**: Allows users to search in loaded packages.
    4.  **Search result**: Only displayed after a search.
3.  Press **Import** to load a package. Sample data can be found under *testdata*.
4.  The loading will start. For large packages this can take some time, but the UI should instantly start to populate the node tree.
5.  When the import is complete, the user is asked if the application should index the attachments. The indexer runs in the background and indexing duration will depend on number of and size of attachments. Windows: At first launch the OS might ask for firewall approval for the *searchd.exe* process, please approve this.
6.  Import generates a report file, this can be opened from the root node of the package.
7.  The application has two main functions:
    1.  **Search**: Free text search in the node tree. When the search starts, the list of search results will appear. Double-click an element in the list to navigate to the element in the node tree. Select a item using the **Select** button. For advances search options press the **+** button next to the **Select** button:
        a) **Attachments**: Only displayed if attachment indexing is complete. When selected attachments will be searched instead of the node tree.
        b) **Case sensitive**: If not checked the search string *findme* will match both *FINDME*, *findMe* and *findme*. If checked it will only match *findme*.
        c) **Include nodes**: Displays all unique node names in the node tree. Nodes not checked will be excluded from the search.
        To hide the advanced search options press the **-** button.
    2.  **Export**: Generate a report of selected nodes in the node tree. The report can be saved to disk or sent as an attachment in e-mail. Please note security requirements for disseminated material before sending on e-mail.


<a id="orgec31a91"></a>

# Sphinx indexer and search engine

For efficient full text searches in the attachments referenced by the archival package the search engine [Sphinx](http://sphinxsearch.com/downloads/sphinx-3.5.1-82c60cb-windows-amd64.zip/thankyou.html) is used. First all attachments are converted to text, then Sphinx builds and index to facilitate efficient look-ups.
The command line tool *pdftotext* is used to convert the PDF files to text. This tool must be avaliable in the path.

With insight running and after loading and indexing an archival package, it is possible to run SQL queries directly on the index with MySQL (version 5.6) client installed:

    mysql -h0 -P9306
    MySQL [(none)]> show tables;
    +------------+-------+
    | Index      | Type  |
    +------------+-------+
    | INDEX_NAME | local |
    +------------+-------+
    1 row in set (0.00 sec)
    
    MySQL [(none)]> select i from INDEX_NAME where match('Drammen');

See Sphinx user manual for more information.


<a id="orgd6db6a1"></a>

# Reports

After import a PDF-report is generated it the report folder as configured by the *REPORTS\_DIR* key in *insight.conf*. The reports are stored in a folder named *REPORTS\_DIR\yyyy\MM\DD\TTMMSS\\*. In the report folder the Sphinx index and similar data attached to the IP is stored.


<a id="orgfdf4cc7"></a>

# Journals

![img](./journal.png)

For some XML based formats there can be a one to many relation between a node in the XML and files in the archival package. An example of this is the Norwegian Health Archive package where the *avlxml.xml* file can reference multiple digitized pages and corresponding OCR metadata. This relationship can be configured using the key *INFO\_VIEW\_JOURNAL\_TYPE\_REGEXP* in the import format file. Nodes matching this key will get a **Journal** button at the bottom of the node view. The Journal view allows users to select pages that should be exported.


<a id="org2ac13b5"></a>

## Creating *searchable-PDFs*

The journal mode supports display and export of journals as *searchable-PDF* where each page consists of the digitized page (for example in JPEG format) an the recognized text (OCR) as an invisible layer.

Supported OCR formats are ALTO and HOCR. For more information how this mode works study the script *pdf\create-pdf.cmd*. To create PDFs several tools have to be installed and available in the system path:

-   python: To run [HocrConverter.py](https://github.com/piql/HocrConverter) responsible for generating PDFs.
-   [pdftk](http://www.pdflabs.com/tools/pdftk-the-pdf-toolkit/): Combines multiple PDFs into one.
-   [alto\_\_hocr.xsl](https://github.com/filak/hOCR-to-ALTO): XSLT scheme for translating from ALTO to hOCR OCR format.


<a id="org96a2449"></a>

# Log files

-   **insight-v1.3.0\insight.log**: Application log.
-   **insight-v1.3.0\insight.dmp**: Generated on Windows if application crashes.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\attachments.log**: Attachments to text convert log.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\indexer.log**: Log from Sphinx indexer.
-   **REPORTS\_DIR\YYYY\MM\DD\TTMMSS\sphinx\test1\searchd.log**: Log from Sphinx search engine.


<a id="org4675467"></a>

# Configuration

The config file is named *insight.conf*. The goal of the config file is that it is self documenting, so inspect it for further details. If the config file is changed the application has to be restarted. Each import format has its own config file stored under *formats*. All files ending with *.conf* in this folder will be loaded at startup and displayed in the file open dialog. 


<a id="orgfe50afe"></a>

## Tip

-   Does the application window appear small and has compact buttons on Windows?
    It could be that the application is running in an emulator or has a screen with high resolution. Try launching the app `insight -platform windows:dpiawareness=0` to get correct window size. Description of other parameters influencing user interface are documented here: <https://doc.qt.io/qt-5/qguiapplication.html#supported-command-line-options>


<a id="org7bca128"></a>

# Batch mode

![img](batch-mode.png)

To support automated dissemination workflows Insight support batch mode using the command line parameters: *&#x2013;file*, *&#x2013;file-format* and *&#x2013;auto-export*. The **auto-export** feature is useful if the format is configures to auto select nodes at import using the *TREEVIEW\_AUTO\_SELECT\_REGEXP* key.


<a id="orga24be88"></a>

## Batch mode example

    insight --file nha-sip.tar --file-format "Norsk Helsearkiv SIP" \
      --auto-export out.pdf

This command will open the AIP file *nha-sip.tar* as a *Norsk Helsearkiv SIP* as defined by <./formats/nha-sip.conf> format file and export it to *out.pdf*, then exit the application. Full example [here](./nha-sip-convert.cmd).

The import process is controlled by various keys in the format file:

    ; File patterns supported by the format, separated by '@'
    IMPORT_FORMAT_PATTERNS=*.7z@*.tar
    ; Extraction tools, order must match pattern list above
    EXTRACT_TOOL="^.*\.7z$@7z x -y \
      -o%DESTINATION% %FILENAME%@^.*\.tar$@tar \
      -C %DESTINATION% -xf %FILENAME%"
    
    ; Auto load nodes will be loaded into tree view when importing XML
    INFOVIEW_AUTO_IMPORT_REGEXP_EN=filename[^.*avlxml\\.xml$]

Auto load nodes will use the first format matching the filename. For the example above this is <./formats/epjark.conf>. This format definition file uses the *TREEVIEW\_AUTO\_SELECT\_REGEXP* key to auto-select nodes at import. Only selected nodes will be included in the export.

    ; Auto select nodes will be selected when importing XML
    TREEVIEW_AUTO_SELECT_REGEXP=pasientjournal@diagnose


<a id="orga83e37a"></a>

# How to report issues

Please create a GitHub issue with a detailed as possible description of what happened. Attach log files and *insight.dmp* if it exists. Do not post sensitive material!


<a id="orgd10e3f4"></a>

# History


<a id="orge1f24fa"></a>

## insight-v1.3.0

-   Upgraded to Qt 6.5
-   Replaced PDF library poppler with QPdfDocument
-   Replaced custom built pdftotext tool with precomiled tool
-   OS-X: Support dark mode
-   OS-X: Fix view relative folder
-   Upgraded to latest sphinx indexer
-   Random format: Bugfixes.
-   Optimize: Made reading & parsing of XMLs run in parallell.
-   Journal: Fixed crash if journal had no pages.

-   Bugs:
    -   OS-X: Does not load config file
    -   Parse Journal pages bug?
    -   Attachments search button not always visible after load.


<a id="org029cca5"></a>

## insight-v1.2.0

-   Translated user manual to English.


<a id="org9eaf462"></a>

## insight-v1.2.0-beta3


<a id="org1c9ba28"></a>

### New features

-   Support for automatic node selection, as defined using the *TREEVIEW\_AUTO\_SELECT\_REGEXP* key.
-   Support for automatic hideing of child nodes using the *TREEVIEW\_AUTO\_COLLAPSE\_REGEXP* key.
-   Journals are included at export.
-   Batch mode: Support automatic export using the command line options: *&#x2013;file*, *&#x2013;file-format* and *&#x2013;auto-export*.


<a id="org0a36f0c"></a>

## innsyn-v1.2.0-beta1


<a id="org0a3e47f"></a>

### New features

-   Support for a fixed export folder, configured using *FIXED\_REPORT\_EXPORT\_FOLDER*.
-   Support journal.


<a id="orgdd72120"></a>

## 01.07.2020 innsyn-v1.1.0


<a id="orgacccf30"></a>

### Fixes

-   Node information: Show all text for nodes and fix spacing issue.
-   Read config files as UTF-8 on Windows.


<a id="org256ce45"></a>

## innsyn-v1.1.0-beta2


<a id="org21fffb0"></a>

### Fixes

-   Install for Windows.


<a id="orgddbb8e5"></a>

## innsyn-v1.1.0-beta1


<a id="orge9e019c"></a>

### New features

-   Support for custom import formats.
-   Support TAR files.
-   Support for loading referenced files.


<a id="org56f71ea"></a>

### Fixes

-   Setup Sphinx for Linux / OS-X.


<a id="org9db0ca6"></a>

## 2018.06.01 innsyn-v1.0.0


<a id="org48e9dc5"></a>

### Fixes

-   Matching for special chars.


<a id="org4651688"></a>

## 2018.04.13 innsyn-v1.0.0-rc1

Pre release.


<a id="org95df22d"></a>

## 2018.02.05 innsyn-v1.0.0-beta2

Pre release.


<a id="org8614bac"></a>

## 2018.01.18 innsyn-v1.0.0-beta1

First beta test release.


<a id="org943f71f"></a>

# Development

The application is created using the Qt framework. When this is installed the application can be build using:

    # Linux / OS-X
    (cd src/thirdparty ; \
     unzip quazip-1.4.zip ; \
     cd quazip-1.4 ; \
     cmake -S . -B ./out -D QUAZIP_QT_MAJOR_VERSION=6 ; \
     cmake --build ./out)
    ./update-translations.sh
    qmake
    make
    
    # Windows
    cd src/thirdparty
    unzip quazip-1.4.zip
    cd quazip-1.4
    cmake -S . -B ./out -D QUAZIP_QT_MAJOR_VERSION=6 -D CMAKE_LIBRARY_PATH=c:\dev\Piql\zlib-win64\Release
    cmake --build ./out --config release 
    cmake --build ./out --config debug
    ./update-translations.sh
    qmake
    nmake

On some systems the MySQL driver has to be built and copied to distribution dir:
<https://doc.qt.io/qt-5/sql-driver.html#qmysql-for-mysql-5-and-higher>

\`\`\`
mkdir mysql
cd mysql
qt-cmake c:\Qt\\6.5.0\Src\qtbase\src\plugins\sqldrivers \\
   -DCMAKE\_INSTALL\_PREFIX=c:\Qt\\6.5.0\msvc2019\_64 \\
   -DMySQL\_INCLUDE\_DIR="c:\Program Files\MySQL\MySQL Server 8.0\include" \\
   -DMySQL\_LIBRARY="c:\Program Files\MySQL\MySQL Server 8.0\lib\libmysql.lib"
\`\`\`

To create release packages use:

    create-release-osx.sh
    create-release.cmd


<a id="org032d1f7"></a>

## pdftotext


<a id="orgc5b69e2"></a>

### Windows

The tool pdftotext comes from the *Xpdf command line tools* pacakge downloaded from <https://www.xpdfreader.com/download.html>. The *pdf2text.exe* and config file *xpdfrc* must be in the path.


<a id="orgb7d9b38"></a>

### Other

Intall with you favourite package manager and ensure tool is avaliable in path.


<a id="orgfe9eeab"></a>

## Ubuntu

    sudo apt install libpoppler*
    sudo apt install libboost-all-dev
    sudo apt install libquazip5-dev
    sudo apt install qttools5-dev-tools


<a id="orge2319ea"></a>

## Windows

    setup-win64.cmd
    qmake -tp vc

