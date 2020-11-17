#!/bin/sh -f

LATEX_DIR=latex-generated
rm -rf $LATEX_DIR
mkdir $LATEX_DIR
TEX_FILES=`find . -iname "*.tex"`
mv $TEX_FILES $LATEX_DIR
cd $LATEX_DIR
pdflatex $TEX_FILES
PDF_FILE=`find . -iname "*.pdf"`
echo $PDF_FILE
mv $PDF_FILE ..
cd ..
evince $PDF_FILE
