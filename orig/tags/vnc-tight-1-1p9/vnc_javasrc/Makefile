#
# Making the VNC applet.
#

CP = cp
JC = javac
JAR = jar
ARCHIVE = vncviewer.jar
PAGES = index.vnc shared.vnc noshared.vnc hextile.vnc zlib.vnc tight.vnc
INSTALL_DIR = /usr/local/vnc/classes

CLASSES = vncviewer.class rfbProto.class authenticationPanel.class \
	  vncCanvas.class optionsFrame.class clipboardFrame.class \
	  animatedMemoryImageSource.class DesCipher.class

SOURCES = vncviewer.java rfbProto.java authenticationPanel.java \
	  vncCanvas.java optionsFrame.java clipboardFrame.java \
	  animatedMemoryImageSource.java DesCipher.java

all: $(CLASSES) $(ARCHIVE)

$(CLASSES): $(SOURCES)
	$(JC) -O $(SOURCES)

$(ARCHIVE): $(CLASSES)
	$(JAR) cf $(ARCHIVE) $(CLASSES)

install: $(CLASSES) $(ARCHIVE)
	$(CP) $(CLASSES) $(ARCHIVE) $(PAGES) $(INSTALL_DIR)

export:: $(CLASSES) $(ARCHIVE) index.vnc shared.vnc
	@$(ExportJavaClasses)

clean::
	$(RM) *.class *.jar
