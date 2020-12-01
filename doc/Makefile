
PANDOC_OPT=--from markdown --pdf-engine=xelatex --template eisvogel.tex --toc --listings --number-sections

all: BrowserSelectorGuide.pdf

%.pdf: %.md eisvogel.tex
	pandoc $< -o $@ $(PANDOC_OPT)

clean:
	rm *.pdf
