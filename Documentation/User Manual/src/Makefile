SOURCES = m32_user-Manual.adoc \
	m32_user-Manual_de.adoc


TARGETS := $(addsuffix .html, $(basename $(SOURCES)))

%.html: %.adoc
	asciidoctor -b html5 $<


all: $(TARGETS)

clean:
	@rm -rf $(TARGETS)
