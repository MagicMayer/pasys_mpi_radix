TEX=pdflatex -8bit -halt-on-error 
BIB=bibtex8
IDX=makeindex
GLS=makeglossaries
RDR=evince

BIN=docu

vpath %.bib	bib

all: pdf clean read

nocopy: pdf clean 
	$(RDR) $(BIN).pdf 1>/dev/null 2>&1 &

pdf:
	clear
	$(TEX) $(BIN) 1>/dev/null 2>&1
# 	$(BIB) $(BIN) 1>/dev/null 2>&1 
# 	$(TEX) $(BIN) 1>/dev/null 2>&1
# 	$(GLS) $(BIN) 1>/dev/null 2>&1
# 	$(TEX) $(BIN) 1>/dev/null 2>&1
# 	$(TEX) $(BIN) 1>/dev/null 2>&1
# 	$(TEX) $(BIN) 1>/dev/null 2>&1

read:
	$(RDR) $(BIN).pdf 1>/dev/null 2>&1 &
	
show: all
	$(RDR) $(BIN).pdf 1>/dev/null 2>&1 &

.PHONY: clean clean-all

clean-all: clean
	rm -f $(BIN).pdf
	rm -f img/*.pdf

clean:
	rm -f $(BIN).aux $(BIN).fls $(BIN).ilg $(BIN).ind  $(BIN).log
	rm -f $(BIN).out $(BIN).toc $(BIN).bcf $(BIN).run.xml $(BIN).bib.blg
	rm -f $(BIN).bbl $(BIN).blg $(BIN)-blx.bib $(BIN).lof $(BIN).lot
	rm -f $(BIN).nlo $(BIN).nls $(BIN).lol $(BIN).glg $(BIN).glo
	rm -f $(BIN).gls $(BIN).ist $(BIN).xdy
	rm -f res/*.aux
