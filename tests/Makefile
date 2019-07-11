always.run: amp.spl
	../sprola -v -l amp.spl
	../sprola -v -l delay.spl

clean:
	rm -f *_out.wav
	rm -f *.ll
	rm -f *.bc
	rm -f *.trace

test:
	cp -r amp.lv2 ~/.lv2/
	cp -r delay.lv2 ~/.lv2/
	lv2file -i test.wav -o amp_test_out.wav --connect=1:inputL --connect=1:inputR http://example.org/sprola/amp
	lv2file -i test.wav -o delay_test_out.wav --connect=1:inputL --connect=1:inputR http://example.org/sprola/delay
