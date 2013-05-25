TEX=pdflatex
MAIN=paal
MAINFILE=$(MAIN).pdf
BIBFILE=$(MAIN).bib
TEXTFILES := $(wildcard *.tex)

all: $(MAINFILE)

.PHONY: $(MAINFILE) all clean

$(MAINFILE): $(BIBFILE) $(TEXFILES)
	$(TEX) $(MAIN)
	bibtex $(MAIN)
	$(TEX) $(MAIN)
	$(TEX) $(MAIN)

clean:
	rm -f *.aux *.log *.err *.bbl *.toc *.blg
