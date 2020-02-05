/* FileChooser.cpp
 * Copyright (C) 2019  Sven Jähnichen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "FileChooser.hpp"
#include <dirent.h>

namespace BWidgets
{
FileChooser::FileChooser () : FileChooser (0.0, 0.0, 0.0, 0.0, "FileChooser") {}

FileChooser::FileChooser (const double x, const double y, const double width, const double height, const std::string& name) :
		FileChooser (x, y, width, height, name, ".", {}, "OK") {}

FileChooser::FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
			  const std::string& path) :
		FileChooser (x, y, width, height, name, path, {}, "") {}

FileChooser::FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
			  const std::string& path, const std::vector<std::string>& filters) :
		FileChooser (x, y, width, height, name, path, filters, "") {}

FileChooser::FileChooser (const double x, const double y, const double width, const double height, const std::string& name,
			  const std::string& path, const std::vector<std::string>& filters, const std::string& buttonText) :
		ValueWidget (x, y, width, height, name, 0.0),
		filters (filters),
		dirs (),
		files (),
		okButtonText (buttonText),
		bgColors (BWIDGETS_DEFAULT_BGCOLORS),
		pathNameBox (0, 0, 0, 0, name + "/textbox", ""),
		fileListBox (0, 0, 0, 0, name + "/listbox"),
		fileNameLabel (0, 0, 0, 0, name + "/label", "File:"),
		fileNameBox (0, 0, 0, 0, name + "/textbox", ""),
		cancelButton (0, 0, 0, 0, name + "/button", "Cancel"),
		okButton (0, 0, 0, 0, name + "/button", buttonText),
		fileFont ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL, 12.0, BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE),
		dirFont ("Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD, 12.0, BStyles::TEXT_ALIGN_LEFT, BStyles::TEXT_VALIGN_MIDDLE)
{
	background_ = BWIDGETS_DEFAULT_MENU_BACKGROUND;
	border_ = BWIDGETS_DEFAULT_MENU_BORDER;
	setDraggable (true);

	setPath (path);
	enterDir();

	pathNameBox.getFont()->setTextAlign (BStyles::TEXT_ALIGN_LEFT);
	pathNameBox.setBackground (BWIDGETS_DEFAULT_MENU_BACKGROUND);
	pathNameBox.setBorder ({{BColors::grey, 1.0}, 0.0, 3.0, 0.0});

	fileNameLabel.getFont()->setTextAlign (BStyles::TEXT_ALIGN_LEFT);

	fileNameBox.setEditable (true);
	fileNameBox.getFont()->setTextAlign (BStyles::TEXT_ALIGN_LEFT);
	fileNameBox.setBackground (BWIDGETS_DEFAULT_MENU_BACKGROUND);
	fileNameBox.setBorder ({{BColors::grey, 1.0}, 0.0, 3.0, 0.0});

	fileListBox.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, fileListBoxClickedCallback);

	cancelButton.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, cancelButtonClickedCallback);
	okButton.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, okButtonClickedCallback);

	add (pathNameBox);
	add (fileListBox);
	add (fileNameLabel);
	add (fileNameBox);
	add (cancelButton);
	add (okButton);
}

FileChooser& FileChooser::operator= (const FileChooser& that)
{
	filters = that.filters;
	dirs = that.dirs;
	files = that.files;
	okButtonText = that.okButtonText;
	bgColors = that.bgColors;
	fileFont = that.fileFont;
	dirFont = that.dirFont;
	pathNameBox = that.pathNameBox;
	fileListBox = that.fileListBox;
	fileNameLabel = that.fileNameLabel;
	fileNameBox = that.fileNameBox;
	cancelButton = that.cancelButton;
	okButton = that.okButton;
	ValueWidget::operator= (that);
	return *this;
}

Widget* FileChooser::clone () const {return new FileChooser (*this);}

void FileChooser::setPath (const std::string& path)
{
	if (path != pathNameBox.getText())
	{
		char buf[PATH_MAX];
		char *rp = realpath(path.c_str(), buf);
		if (rp) pathNameBox.setText (rp);
		else pathNameBox.setText (path);

		update();
	}
}

std::string FileChooser::getPath () const {return pathNameBox.getText();}

std::string FileChooser::getFileName () const {return fileNameBox.getText();}

void FileChooser::setFilters (const std::vector<std::string>& filters)
{
	if (filters != this->filters)
	{
		this->filters = filters;
		update ();
	}
}

std::vector<std::string> FileChooser::getFilters () const {return filters;}

void FileChooser::setButtonText (const std::string& buttonText)
{
	if (buttonText != okButtonText)
	{
		okButtonText = buttonText;
		update();
	}
}

std::string FileChooser::getButtonText () {return okButtonText;}

// TODO calculate minimal size
void FileChooser::resize () {resize (BWIDGETS_DEFAULT_FILECHOOSER_WIDTH, BWIDGETS_DEFAULT_FILECHOOSER_HEIGHT);}

void FileChooser::resize (const double width, const double height) {resize (BUtilities::Point (width, height));}

void FileChooser::resize (const BUtilities::Point extends) {Widget::resize (extends);}

void FileChooser::update ()
{
	double x0 = getXOffset();
	double y0 = getYOffset();
	double w = getEffectiveWidth();
	double h = getEffectiveHeight();

	if ((w >= 20) && (h >= 20))
	{
		double val = fileListBox.getValue();
		if ((val == UNSELECTED) || (val > dirs.size())) okButton.getLabel()->setText (okButtonText);
		else okButton.getLabel()->setText ("Open");

		// Get extends first
		okButton.resize();
		cancelButton.resize ();
		double okWidth = (okButton.getWidth() > cancelButton.getWidth() ? okButton.getWidth() : cancelButton.getWidth());
		double okHeight = (okButton.getHeight() > cancelButton.getHeight() ? okButton.getHeight() : cancelButton.getHeight());
		pathNameBox.resize();
		double pathNameHeight = pathNameBox.getHeight();
		fileNameBox.resize();
		double fileNameHeight = fileNameBox.getHeight();
		fileNameLabel.resize();
		double fileNameWidth = fileNameLabel.getWidth();

		pathNameBox.moveTo (x0 + 10, y0 + 10);
		pathNameBox.resize (w - 20, pathNameHeight);

		okButton.moveTo (x0 + w - okWidth - 10, y0 + h - okHeight - 10);
		okButton.resize (okWidth, okHeight);

		cancelButton.moveTo (x0 + 10, y0 + h - okHeight - 10);
		cancelButton.resize (okWidth, okHeight);

		fileNameLabel.moveTo (x0 + 10, y0 + h - okHeight - fileNameHeight - 20);
		fileNameLabel.resize (fileNameWidth, fileNameHeight);

		fileNameBox.moveTo (x0 + fileNameWidth + 30, y0 + h - okHeight - fileNameHeight - 20);
		fileNameBox.resize (w - fileNameWidth - 40, fileNameHeight);

		okButton.show();
		cancelButton.show();
		fileNameLabel.show();
		fileNameBox.show();

		if (h > pathNameHeight + okHeight + fileNameHeight + 50)
		{
			fileListBox.moveTo (x0 + 10, y0 + pathNameHeight + 20);
			fileListBox.resize (w - 20, h - pathNameHeight - okHeight - fileNameHeight - 50);
			fileListBox.resizeItems (BUtilities::Point (fileListBox.getEffectiveWidth(), 20));
			fileListBox.show();
		}
		else fileListBox.hide();
	}

	else
	{
		okButton.hide();
		cancelButton.hide();
		fileListBox.hide();
		fileNameLabel.hide();
		fileNameBox.hide();
	}

	Widget::update();
}

void FileChooser::applyTheme (BStyles::Theme& theme) {applyTheme (theme, name_);}

void FileChooser::applyTheme (BStyles::Theme& theme, const std::string& name)
{
	cancelButton.applyTheme (theme, name + "/button");
	okButton.applyTheme (theme, name + "/button");
	fileListBox.applyTheme (theme, name + "/listBox");
	Widget::applyTheme (theme, name);

	// Color
	void* bgPtr = theme.getStyle(name, BWIDGETS_KEYWORD_BGCOLORS);
	if (bgPtr) bgColors = *((BColors::ColorSet*) bgPtr);

	// TODO Store dir and file styles

	if (bgPtr) update ();
}

void FileChooser::fileListBoxClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	ListBox* w = (ListBox*)event->getWidget();
	if (!w) return;
	FileChooser* fc = (FileChooser*)w->getParent();
	if (!fc) return;

	double val = w->getValue();

	if ((val != UNSELECTED) && (!fc->fileNameBox.getEditMode()))
	{
		// Directory selected
		if (val <= fc->dirs.size()) fc->fileNameBox.setText ("");

		// File selected
		else
		{
			BItems::Item* ai = w->getActiveItem();
			if (ai)
			{
				Label* ail = (Label*)ai->getWidget();
				if (ail) fc->fileNameBox.setText (ail->getText());
			}
		}

		fc->update();
	}

	// TODO Double clicked
}

void FileChooser::cancelButtonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	TextButton* w = (TextButton*)event->getWidget();
	if (!w) return;
	FileChooser* fc = (FileChooser*)w->getParent();
	if (!fc) return;

	BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*)event;
	if (ev->getValue() == 1.0)
	{
		fc->setValue (0.0);
		fc->postCloseRequest();
	}
}

void FileChooser::okButtonClickedCallback (BEvents::Event* event)
{
	if (!event) return;
	TextButton* w = (TextButton*)event->getWidget();
	if (!w) return;
	FileChooser* fc = (FileChooser*)w->getParent();
	if (!fc) return;

	BEvents::ValueChangedEvent* ev = (BEvents::ValueChangedEvent*)event;
	if (ev->getValue() == 1.0)
	{
		double lb = fc->fileListBox.getValue();

		if (lb != UNSELECTED)
		{
			// Dircectory selected: Open directory
			if (lb <= fc->dirs.size())
			{
				BItems::Item* ai = fc->fileListBox.getActiveItem();
				if (ai)
				{
					Label* ail = (Label*)ai->getWidget();
					if (ail)
					{
						std::string newPath = fc->getPath() + "/" + ail->getText();
						char buf[PATH_MAX];
				    		char *rp = realpath(newPath.c_str(), buf);
				    		if (rp) fc->setPath (rp);

						fc->enterDir();
						fc->update();
					}
				}
			}

			// File selected: OK on file
			else
			{
				fc->setValue (1.0);
				fc->postCloseRequest();
			}
		}
	}
}

void FileChooser::enterDir ()
{
	std::vector<std::string> newFiles;
	std::vector<std::string> newDirs;
	DIR *dir = opendir (getPath().c_str());

	// Scan directory
	// TODO Use C++17 for future versions
	if (dir)
	{
		for (struct dirent* entry = readdir(dir); entry ; entry = readdir(dir))
		{
			if (entry->d_type == DT_DIR)
			{
				newDirs.push_back (entry->d_name);
			}

			else
			{
				std::string s = entry->d_name;

				if (filters.size() == 0) newFiles.push_back (s);
				else
				{
					for (std::string const& f : filters)
					{
						if (s.find (f) != std::string::npos)
						{
							newFiles.push_back (s);
							break;
						}
					}
				}
			}
		}
		closedir (dir);
	}

	std::sort (newFiles.begin(), newFiles.end());
	std::sort (newDirs.begin(), newDirs.end());

	if ((files != newFiles) || (dirs != newDirs))
	{
		files = newFiles;
		dirs = newDirs;

		BItems::ItemList items;
		size_t count = 1;

		// Directories
		for (std::string const& d : dirs)
		{
			BItems::Item item = BItems::Item (count, d);
			Widget* widget = item.getWidget();
			if (widget)
			{
				((Label*)widget)->setFont (dirFont);
				widget->rename (getName() + "/listBox/item/dir");
				items.push_back (item);
				++count;
			}
		}

		// Files
		for (std::string const& f : files)
		{
			BItems::Item item = BItems::Item (count, f);
			Widget* widget = item.getWidget();
			if (widget)
			{
				((Label*)widget)->setFont (fileFont);
				widget->rename (getName() + "/listBox/item/file");
				items.push_back (item);
				++count;
			}
		}

		// Set new listbox
		fileListBox = ListBox (fileListBox.getPosition().x, fileListBox.getPosition().y,
				       fileListBox.getWidth(), fileListBox.getHeight(),
			       	       fileListBox.getName(), items);
		fileListBox.setCallbackFunction (BEvents::VALUE_CHANGED_EVENT, fileListBoxClickedCallback);
	}
}

}
