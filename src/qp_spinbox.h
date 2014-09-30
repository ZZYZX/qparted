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

/* About QP_SpinBox class:
 *
 * This is a widget derived from QSpinBox and it implement a trivial spinbox that can
 * handle decimal values
 *
 * Please note that this is a very TRIVIAL implementation! I'll clean up the code in
 * a next release! ;)
 */

#include <qspinbox.h>
#include <qvalidator.h>

#ifndef QP_SPINBOX_H
#define QP_SPINBOX_H

#define MEGABYTE (1024 * 1024)
#define GIGABYTE (1024 * 1024 * 1024)
#define MBYTE_SECTORS (MEGABYTE / 512)
#define GBYTE_SECTORS (GIGABYTE / 512)

typedef long long PedSector;

class QP_SpinBox : public QDoubleSpinBox {
	Q_OBJECT
public:
	QP_SpinBox(QWidget *parent = 0L);
	~QP_SpinBox();

	void setFormatString(const char *fmt);

	void setValue(PedSector);
	void setMaxValue(PedSector);
	void setMinValue(PedSector);

	void setStep(float step);
	float getStep() const;

	void setRange(PedSector minVal, PedSector maxVal);
	void getRange(float &minVal, float &maxVal);
	PedSector pedValue();
	PedSector pedMaxValue();

signals:
	void valueChanged(PedSector);

public slots:
	virtual void stepUp();
	virtual void stepDown();
	void setMaxValue(float);
	void setMinValue(float);
	void setFormat(int);

protected:
	int mapTextToValue(bool *ok);
	QString mapValueToText(int v);
	void valueChange();

private:
	void setRange(float minVal, float maxVal);
	void setValue(float value);
	float step, _floatminval, _floatmaxval, _floatvalue;
	QString format;
	QDoubleValidator *val;
	PedSector _pedminval;
	PedSector _pedmaxval;
	PedSector _pedvalue;
	int _format;
	bool _update;
};
#endif
