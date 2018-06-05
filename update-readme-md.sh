#!/bin/sh

emacs lesmeg.org --batch -f org-md-export-to-markdown
mv lesmeg.md README.md
