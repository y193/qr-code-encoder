CC = clang
CFLAGS = -Wall -Wextra -O2 -I./src

.PHONY: qrce
qrce: bin bin/qrce.exe

.PHONY: test
test: bin \
      bin/test_charset.exe \
      bin/test_dataanalysis.exe \
      bin/test_dataencoding.exe \
      bin/test_errorcorrection.exe \
      bin/test_finalmessage.exe \
      bin/test_moduleplacement.exe \
      bin/test_datamasking.exe \
      bin/test_formatandversion.exe \
      bin/test_gf256.exe

.PHONY: all
all: bin qrce test

bin/qrce.exe: bin/main.o \
			  bin/charset.o \
			  bin/gf256.o \
			  bin/rsblock.o \
			  bin/segment.o \
              bin/dataanalysis.o \
			  bin/dataencoding.o \
			  bin/errorcorrection.o \
			  bin/finalmessage.o \
			  bin/moduleplacement.o \
			  bin/datamasking.o \
			  bin/formatandversion.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_charset.exe: bin/charset.o bin/test_charset.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_gf256.exe: bin/gf256.o bin/test_gf256.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_dataanalysis.exe: bin/charset.o bin/segment.o bin/dataanalysis.o bin/test_dataanalysis.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_dataencoding.exe: bin/charset.o bin/segment.o bin/dataencoding.o bin/test_dataencoding.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_errorcorrection.exe: bin/gf256.o bin/errorcorrection.o bin/test_errorcorrection.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_finalmessage.exe: bin/finalmessage.o bin/test_finalmessage.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_moduleplacement.exe: bin/test_module.o bin/moduleplacement.o bin/test_moduleplacement.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_datamasking.exe: bin/test_module.o bin/datamasking.o bin/test_datamasking.o
	${CC} $(LDFLAGS) -o $@ $^

bin/test_formatandversion.exe: bin/test_module.o bin/formatandversion.o bin/test_formatandversion.o
	${CC} $(LDFLAGS) -o $@ $^

bin/%.o: src/%.c
	${CC} ${CFLAGS} -c $< -o $@

bin/%.o: test/%.c
	${CC} ${CFLAGS} -c $< -o $@

.PHONY: bin
bin:
	CMD /C "IF NOT EXIST bin (MKDIR bin)"

.PHONY: clean
clean:
	CMD /C "DEL /Q bin\*.o bin\*.exe"
