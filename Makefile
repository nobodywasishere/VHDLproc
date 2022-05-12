
clean:
	rm -rf build/ dist/ vhdlproc.egg-info vhdlproc/__pycache__ vhdlproc/tests/*.proc.vhdl

test:
	./vhdlproc/vhdlproc.py --self-test

format:
	black .

dist:
	python setup.py sdist bdist_wheel
	twine upload dist/*

.PHONY: clean test format dist
