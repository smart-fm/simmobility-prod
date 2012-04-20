UI_PYTHON_FILES = \
    ui_window.py \
    ui_edit_form.py \
    ui_multiple_edit_form.py \

RESOURCE_FILE = pqrne_rc.py

all: $(UI_PYTHON_FILES) $(RESOURCE_FILE)

ui_window.py: ui-files/window.ui
	pyuic4 -o $@ $?

ui_edit_form.py: ui-files/edit_form.ui
	pyuic4 -o $@ $?

ui_multiple_edit_form.py: ui-files/multiple_edit_form.ui
	pyuic4 -o $@ $?

pqrne_rc.py: ui-files/pqrne.qrc
	pyrcc4 -o $@ $?

clean:
	rm -f *~ $(UI_PYTHON_FILES) $(RESOURCE_FILE) *.pyc
