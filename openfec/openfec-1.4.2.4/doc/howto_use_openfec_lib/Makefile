# initialize the NAME_BASE variable with the file name without any extension
NAME_BASE=howto_use_openfec_lib
TEX=${NAME_BASE}.tex
DVI=${NAME_BASE}.dvi
PS=${NAME_BASE}.ps
PS2P=${NAME_BASE}_2p.ps
PDF=${NAME_BASE}.pdf
PS_GZ=${PS}.gz
PS2P_GZ=${PS2P}.gz
RTF=${NAME_BASE}.rtf


# classic version...
LATEX=latex
# when pdfetex is used as an alias to latex...
#LATEX=latex -output-format dvi


all:	pdf

ps:
	${LATEX} ${TEX}
	dvips ${DVI} -o ${PS} -Ppdf -G0
	if [ -e "${PS_GZ}" ] ; then \
		rm "${PS_GZ}"; \
	fi
	gzip ${PS}

biblio:
	#export BIBINPUTS=bib/
	export BIBINPUTS=.
	bibtex ${NAME_BASE}
	latex ${TEX}
	latex ${TEX}

html:
	latex2html -split 0 ${NAME_BASE}

ps2p:
	${LATEX} ${TEX}
	dvips ${DVI} -o ${PS} -Ppdf -G0
	psnup -2 ${PS} > ${PS2P}
	if [ -e "${PS_GZ}" ] ; then \
		rm "${PS_GZ}"; \
	fi
	gzip ${PS}
	if [ -e "${PS2P_GZ}" ] ; then \
		rm "${PS2P_GZ}"; \
	fi
	gzip ${PS2P}

# Use it if pdfetex produces PDF directly...
#pdf:
#	${LATEX} ${TEX}

pdf:
	${LATEX} ${TEX}
	dvips ${DVI} -o ${PS} -Ppdf -G0
	ps2pdf ${PS}
	rm ${PS}

rtf:
	latex2rtf -Z9 ${TEX}

clean:
	rm -f ${NAME_BASE}.aux ${NAME_BASE}.log ${NAME_BASE}.toc ${NAME_BASE}.bbl ${NAME_BASE}.blg ${NAME_BASE}.out ${NAME_BASE}.dvi

cleanall: clean
	if [ -e "${PS_GZ}" ] ; then \
		rm "${PS_GZ}"; \
	fi
	if [ -e "${PS}" ] ; then \
		rm "${PS}"; \
	fi
	if [ -e "${PS2P}" ] ; then \
		rm "${PS2P}"; \
	fi
	if [ -e "${DVI}" ] ; then \
		rm "${DVI}"; \
	fi
	if [ -e "${PDF}" ] ; then \
		rm "${PDF}"; \
	fi
	if [ -e "${RTF}" ] ; then \
		rm "${RTF}"; \
	fi
