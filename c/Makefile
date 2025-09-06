DEPFILES = main.c jpegdec.c
DEPEND = Depend
FF16 = ff16/source
XPRINTF = xprintf/src
PICOJPEG = picojpeg
CFLAGS = -mr800 --less-pedantic -I. -I$(FF16) -I$(XPRINTF) -I$(PICOJPEG)
VPATH = $(FF16):$(XPRINTF):$(PICOJPEG)

all: $(DEPEND) a.bin

a.bin: main.lk crt0.rel main.rel xprintf.rel ff.rel diskio.rel \
		picojpeg.rel jpegdec.rel sjpeg.rel
	sdldz80 -nf main.lk
	makebin -p a.ihx a.bin
	z80dasm -l -t -g 0 a.bin > a.lst
	./mkadr.pl *.sym > a.adr

$(DEPEND):
	rm -f $(DEPEND)
	for file in $(DEPFILES); do sdcc $(CFLAGS) -MM $$file | sed -e 's/\.o:/.rel:/' >> $(DEPEND); done

clean:
	rm -f $(DEPEND) a.bin
	rm -f *.{adr,asm,ihx,lst,map,noi,rel,sym}

%.rel: %.c
	sdcc -c $(CFLAGS) $<
%.rel: %.s
	sdasz80 -lsw -o $@ $<

-include $(DEPEND)
