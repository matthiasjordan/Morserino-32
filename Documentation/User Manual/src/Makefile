SOURCES = m32_user-Manual.adoc \
	m32_user-Manual_de.adoc


TARGETS := $(addsuffix .html, $(basename $(SOURCES))) \
	$(addsuffix .pdf, $(basename $(SOURCES)))

%.html: %.adoc
	asciidoctor -b html5 $<

%.pdf: %.adoc
	asciidoctor-pdf -a imagesdir=Images -a pdf-themesdir=Images $<


all: $(TARGETS)

clean:
	@rm -rf $(TARGETS)
