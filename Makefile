all: sockfile.jar

sockfile.jar: META-INF/MANIFEST.MF src/main/java/org/spacehq/sockfile/Main.class
	@jar cmvf META-INF/MANIFEST.MF $@ -C src/main/java org/spacehq/sockfile/Main.class

src/main/java/org/spacehq/sockfile/Main.class: src/main/java/org/spacehq/sockfile/Main.java
	@javac -classpath src/main/java src/main/java/org/spacehq/sockfile/Main.java

clean:
	@$(RM) sockfile.jar src/main/java/org/spacehq/sockfile/Main.class
