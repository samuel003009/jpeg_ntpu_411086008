INCLUDE = -I.

all: build_encoder build_decoder

build_encoder:
	gcc -o encoder_411086008.exe encoder_411086008.c -lm

build_decoder:
	gcc -o decoder_411086008.exe decoder_411086008.c -lm

demo0:
	./encoder_411086008.exe 0 KimberlyNCat.bmp R.txt G.txt B.txt dim.txt
	./decoder_411086008.exe 0 ResKimberlyNCat.bmp R.txt G.txt B.txt dim.txt
demo1:
	./encoder_411086008.exe 1 KimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw
	./decoder_411086008.exe 1 QResKimberlyNCat.bmp KimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw
	./decoder_411086008.exe 1 ResKimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw

demo2:
	./encoder_411086008.exe 2 KimberlyNCat.bmp ascii rle_code.txt
	./decoder_411086008.exe 2 QResKimberlyNCat.bmp ascii rle_code.txt
clean:
	rm -f encoder_411086008.exe decoder_411086008.exe R.txt G.txt B.txt dim.txt
	rm -f Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw
	rm -f rle_code.txt

.PHONY: build_encoder build_decoder demo0 clean
