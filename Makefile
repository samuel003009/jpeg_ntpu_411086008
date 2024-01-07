INCLUDE = -I.

all: build_encoder build_decoder

build_encoder:
	gcc -o encoder.exe encoder.c -lm

build_decoder:
	gcc -o decoder.exe decoder.c -lm

demo0:
	./encoder.exe 0 KimberlyNCat.bmp R.txt G.txt B.txt dim.txt
	./decoder.exe 0 ResKimberlyNCat.bmp R.txt G.txt B.txt dim.txt
demo1:
	./encoder.exe 1 KimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw
	./decoder.exe 1 QResKimberlyNCat.bmp KimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw
	./decoder.exe 1 ResKimberlyNCat.bmp Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw

clean:
	rm -f encoder.exe decoder.exe R.txt G.txt B.txt dim.txt
	rm -f Qt_Y.txt Qt_Cb.txt Qt_Cr.txt dim.txt qF_Y.raw qF_Cb.raw qF_Cr.raw eF_Y.raw eF_Cb.raw eF_Cr.raw

.PHONY: build_encoder build_decoder demo0 clean
