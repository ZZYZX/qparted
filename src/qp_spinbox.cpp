/*
    qtparted - a frontend to libparted for manipulating disk partitions
    Copyright (C) 2002-2003 Vanni Brutto

    Vanni Brutto <zanac (-at-) libero dot it>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <cstdlib>
#include <cstdio>

#include "qp_spinbox.h"

QP_SpinBox::QP_SpinBox(QWidget *parent)
	:QDoubleSpinBox (parent)
	,_floatminval(0)
	,_floatmaxval(0)
	,_pedminval(0)
	,_pedmaxval(0)
	,step(1)
	,_floatvalue(0)
	,_format(0)
	,_update(true)
{
	/*---format of the displayed string (ie: 999.99)---*/
	format = "%3.2f";
	QDoubleSpinBox::setValue(0.0);
	setDecimals(2);
	setMinimum(_floatminval);
	setMaximum(_floatmaxval);
}


QP_SpinBox::~QP_SpinBox() {
}


void QP_SpinBox::setFormatString (const char *fmt) {
	format = fmt;
}


/*---change between mbyte and gbyte---*/
void QP_SpinBox::setFormat(int format) {
	_format = format;
	float f_minvalue = 0;
	float f_maxvalue = 0;
	float f_value = 0;

	if (_format == 0) {
		f_minvalue = float(_pedminval * 1.0 / MBYTE_SECTORS);
		f_maxvalue = float(_pedmaxval * 1.0 / MBYTE_SECTORS);
		f_value = float(_pedvalue * 1.0 / MBYTE_SECTORS);
	} else if (_format == 1) {
		f_minvalue = float(_pedminval * 1.0 / GBYTE_SECTORS);
		f_maxvalue = float(_pedmaxval * 1.0 / GBYTE_SECTORS);
		f_value = float(_pedvalue * 1.0 / GBYTE_SECTORS);
	}

	/*---don't update internal ped values!---*/
	_update = false;
	setRange(f_minvalue, f_maxvalue);
	setValue(f_value);
	_update = true;
}

void QP_SpinBox::setValue(float value) {
	char buf[20];
	if (_floatminval <= value && value <= _floatmaxval) {
		sprintf (buf, format.toLatin1(), value);
		_floatvalue = value;
		QDoubleSpinBox::setValue(int(value * 100.0));
	}
}


void QP_SpinBox::setValue(PedSector value) {
	if (_pedminval <= value && value <= _pedmaxval) {
		float f_value = 0;

		if (_format == 0)
			f_value = float(value * 1.0 / MBYTE_SECTORS); else
		if (_format == 1)
			f_value = float(value * 1.0 / GBYTE_SECTORS);

		_pedvalue = value;

		/*---don't update internal ped values!---*/
		_update = false;
		setValue(f_value);
		_update = true;
	}
}


void QP_SpinBox::setMaxValue(PedSector maxVal) {
	float f_maxvalue = 0;

	_pedmaxval = maxVal;

	if (_format == 0)
		f_maxvalue = float(maxVal * 1.0 / MBYTE_SECTORS);
	else if (_format == 1)
		f_maxvalue = float(maxVal * 1.0 / GBYTE_SECTORS);

	setMaxValue(f_maxvalue);
}


void QP_SpinBox::setMinValue(PedSector minVal) {
	float f_minvalue = 0;

	_pedminval = minVal;

	if (_format == 0)
		f_minvalue = float(minVal * 1.0 / MBYTE_SECTORS);
	else if (_format == 1)
		f_minvalue = float(minVal * 1.0 / GBYTE_SECTORS);

	setMinValue(f_minvalue);
}


void QP_SpinBox::setStep (float s) {
	step = s;
	//setSteps ((int)(step * 10.0), (int)(step * 10.0));
}

float QP_SpinBox::getStep() const {
	return step;
}

void QP_SpinBox::setRange(float minVal, float maxVal) {
	if (minVal <= maxVal) {
		_floatminval = minVal;
		_floatmaxval = maxVal;
		setRange(minVal, maxVal);
		val->setRange(minVal, maxVal, 2);
	}
}


void QP_SpinBox::setRange(PedSector minVal, PedSector maxVal) {
	float f_minvalue = 0;
	float f_maxvalue = 0;

	_pedminval = minVal;
	_pedmaxval = maxVal;

	if (_format == 0) { f_minvalue = float(minVal * 1.0 / MBYTE_SECTORS);
						f_maxvalue = float(maxVal * 1.0 / MBYTE_SECTORS); } else
	if (_format == 1) { f_minvalue = float(minVal * 1.0 / GBYTE_SECTORS);
						f_maxvalue = float(maxVal * 1.0 / GBYTE_SECTORS); }

	QDoubleSpinBox::setRange(f_minvalue * 100.0, f_maxvalue * 100.0);
}


void QP_SpinBox::getRange(float &minVal, float &maxVal) {
	minVal = _floatminval;
	maxVal = _floatmaxval;
}

PedSector QP_SpinBox::pedValue() {
	return _pedvalue;
}

PedSector QP_SpinBox::pedMaxValue() {
	return _pedmaxval;
}

void QP_SpinBox::stepUp() {
	QDoubleSpinBox::stepUp();

	int diff = int(_floatmaxval * 100.0) - int(_floatvalue * 100.0);
	if (diff <= 1) _pedvalue = _pedmaxval;
}

void QP_SpinBox::stepDown() {
	QDoubleSpinBox::stepDown();

	int diff = int(_floatvalue * 100.0) - int(_floatminval * 100.0);
	if (diff <= 1) _pedvalue = _pedminval;
}

void QP_SpinBox::setMaxValue(float maxvalue) {
	_floatmaxval = maxvalue;
	setMaximum(maxvalue * 100.0);
}

void QP_SpinBox::setMinValue(float minvalue) {
	_floatminval = minvalue;
	setMinimum(minvalue * 100.0);
}

void QP_SpinBox::valueChange() {
	bool rc;
	//updateDisplay();
	mapTextToValue(&rc);

	/*---don't update internal ped values!---*/
	if (_update) {
		if (_floatvalue == _floatminval)
			_pedvalue = _pedminval;
		else if (_floatvalue == _floatmaxval)
			_pedvalue = _pedmaxval;
		else {
			if (_format == 0)
				_pedvalue = PedSector(_floatvalue * MBYTE_SECTORS);
			else if (_format == 1)
				_pedvalue = PedSector(_floatvalue * GBYTE_SECTORS);
		}
	}

	/*---ped value should be beetween min and max---*/
	if (_pedvalue < _pedminval)
		_pedvalue = _pedminval;

	if (_pedvalue > _pedmaxval)
		_pedvalue = _pedmaxval;

	emit valueChanged(_pedvalue);
}


int QP_SpinBox::mapTextToValue(bool *ok) {
	const char *txt = text().toLatin1();
	_floatvalue = atof(txt);
	*ok = true;
	return int(_floatvalue * 100.0);
}


QString QP_SpinBox::mapValueToText(int v) {
	float f = float(v) / 100.0;
	QString buf;
	buf.sprintf(format.toLatin1(), f);
	return buf;
}
