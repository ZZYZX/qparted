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

/* About QP_ListExternalTools class
 *
 * This class keep a list of all external tools needed by libparted wrapper
 */


#ifndef QP_EXTTOOLS_H
#define QP_EXTTOOLS_H

#include <QHash>
#include <QString>

class QP_ExternalTool {
public:
    QP_ExternalTool() {}
    QP_ExternalTool(QString name, QString path, QString description) {
        _name = name;
        _path = path;
        _description = description;
    }
    QString path() {return _path;}
    QString description() {return _description;}
    QString name() {return _name;}
    void setPath(QString path) {_path = path;}
    void setDescription(QString description) {_description = description;}
    void setName(QString name) {_name = name;}

private:
    QString _path;
    QString _description;
    QString _name;
};

class QP_ListExternalTools:public QHash<QString,QP_ExternalTool*> {
public:
    QP_ListExternalTools();
    ~QP_ListExternalTools();
    void add(QString name, QString path, QString description);
    QString getPath(QString);
    void setPath(QString, QString);
    QString getDescription(QString);
};

#endif
