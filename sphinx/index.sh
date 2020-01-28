#!/bin/sh

REPORTS=$1
ATTACHMENTS=$2
NAME=$3
CONFIG="$REPORTS/sphinx.conf"
INDEX_CONFIG="$REPORTS/sphinx_index.conf"
INDEX="$REPORTS""sphinx/test1"
SOURCE_NAME=src_$NAME
INDEX_NAME=$NAME
SOURCE_CONFIG="$(dirname $0)/sphinx_index.conf"
BASE_CONFIG="$(dirname $0)/sphinx.conf"

[ ! -f $SOURCE_CONFIG ] && { echo "Not found: $SOURCE_CONFIG" ; exit 1; }
[ ! -f $BASE_CONFIG ] && { echo "Not found: $BASE_CONFIG" ; exit 1; }
which -s indexer || { echo "Sphinx indexer tool needed" ; exit 1; }
which -s create_xml || { echo "create_xml tool needed"; exit 1; }


mkdir -p $INDEX

# Create sphinx config file in reports folder
rm $INDEX_CONFIG 2> /dev/null
cp $SOURCE_CONFIG $INDEX_CONFIG
sed -i -e "s|SOURCE_DIR|$ATTACHMENTS|g" $INDEX_CONFIG
sed -i -e "s|SOURCE_DIR|$SOURCE_NAME|g" $INDEX_CONFIG
sed -i -e "s|SOURCE_DIR|$INDEX_DIR|g"   $INDEX_CONFIG
sed -i -e "s|SOURCE_DIR|$INDEX_NAME|g"  $INDEX_CONFIG


more $INDEX_CONFIG > $CONFIG
more $BASE_CONFIG >> $CONFIG

# Run indexer
indexer --config $CONFIG --all > $REPORTS/indexer.log
