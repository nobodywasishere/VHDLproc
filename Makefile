
clean:
	rm -rf build/ dist/ vhdlproc.egg-info vhdlproc/__pycache__ vhdlproc/tests/*.proc.vhdl

test:
	./vhdlproc/vhdlproc.py --self-test
	./vhdlproc/vhdlproc.py vhdlproc/tests/and.vhdl
	./vhdlproc/vhdlproc.py vhdlproc/tests/and.proc.vhdl --parse-comments
	./vhdlproc/vhdlproc.py vhdlproc/tests/include.vhdl
	./vhdlproc/vhdlproc.py vhdlproc/tests/include.proc.vhdl --parse-comments
	./vhdlproc/vhdlproc.py vhdlproc/tests/nest.vhdl
	./vhdlproc/vhdlproc.py vhdlproc/tests/nest.proc.vhdl --parse-comments

format:
	black .

dist:
	python setup.py sdist bdist_wheel
	twine upload dist/*

.PHONY: clean test format dist
