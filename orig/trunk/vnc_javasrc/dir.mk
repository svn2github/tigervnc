#
# Making the VNC applet.
#

CLASSES = vncviewer.class rfbProto.class authenticationPanel.class \
	  vncCanvas.class optionsFrame.class clipboardFrame.class \
	  DesCipher.class

all: $(CLASSES) vncviewer.jar

vncviewer.jar: $(CLASSES)
	@$(JavaArchive)

export:: $(CLASSES) vncviewer.jar index.vnc shared.vnc
	@$(ExportJavaClasses)

clean::
	$(RM) *.class *.jar
