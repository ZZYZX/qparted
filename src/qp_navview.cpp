/*
    qtparted - a frontend to libparted for manipulating disk partitions
    Copyright (C) 2002-2003 Vanni Brutto <zanac (-at-) libero dot it>
    Copyright (C) 2007-2008 David Tio <deux (-at-) arklinux dot org >

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

#include <QLayout>
#include <QPalette>
#include "qp_navview.h"

QP_NavView::QP_NavView ( QWidget *parent, QP_Settings *settings )
        : QWidget ( parent )
{

    QVBoxLayout *box = new QVBoxLayout ( this );
    box->setMargin ( 6 );
    box->setSpacing ( 6 );

    /*---Drive List on the left (it display a tree with hda, hdb /etc /etc) ---*
     *---it is comped by:                                                   ---*
     *---                                                                   ---*
     *---   .title                                                          ---*
     *---   .drivelist                                                      ---*
     *---        * /dev/hda                                                 ---*
     *---        * /dev/hdb                                                 ---*
     *---   .driveinfo                                                      ---*
     *---        *infotitle                                                 ---*
     *---        *details                                                   ---*/

    //TITLE
    QLabel *title = new QLabel ( tr ( "The following drives have been detected:" ), this );
    box->addWidget ( title );

    //DRIVELIST
    drivelist = new QP_DriveList ( this, settings );
    connect ( drivelist, SIGNAL ( deviceSelected ( QP_Device * ) ),
              this, SIGNAL ( deviceSelected ( QP_Device * ) ) );
    box->addWidget ( drivelist );

    connect ( drivelist, SIGNAL ( deviceSelected ( QP_Device * ) ),
              this, SLOT ( displayInfo ( QP_Device * ) ) );

    //DRIVEINFO

    QFrame *infoBox = new QFrame(this);
    infoBox->setFrameStyle ( QFrame::Box | QFrame::Plain );
    QPalette ibp ( infoBox->palette() );
    ibp.setColor ( infoBox->backgroundRole(), Qt::black );
    infoBox->setPalette ( ibp );
    infoBox->setAutoFillBackground ( true );

    QVBoxLayout *infoBoxLayout = new QVBoxLayout(infoBox);
    infoBoxLayout->setMargin ( 2 );
    infoBoxLayout->setSpacing ( 2 );

    QLabel *infoTitle = new QLabel ( "<qt><center><b>" + tr ( "Drive Info" ) + "</b></center></qt>", infoBox );
    QPalette itp ( infoTitle->palette() );
    itp.setColor ( infoTitle->backgroundRole(), QColor ( 0xa8, 0xa8, 0xff ) );
    infoTitle->setPalette ( itp );
    infoTitle->setAutoFillBackground ( true );
    details = new QLabel(infoBox);

    infoBoxLayout->addWidget ( infoTitle );
    infoBoxLayout->addWidget ( details );

    infoBox->setLayout ( infoBoxLayout );

    box->addWidget ( infoBox );

    /*---init driveinfo details---*/
    displayInfo ( NULL );
}

QP_NavView::~QP_NavView()
{
}

void QP_NavView::setPopup ( QMenu *popup )
{
    drivelist->setPopup ( popup );
}

void QP_NavView::init()
{
    drivelist->buildView();
}

/*---fill the driveinfo details with info about the hard disk selected---*/
void QP_NavView::displayInfo ( QP_Device *device )
{
    QString infoTemplate ( "<qt bgcolor=\"#c8c8ff\">"
                           "<table width=100% bgcolor=\"#c8c8ff\" cellspacing=0 cellpadding=3>%1</table>"
                           "</qt>" );

    if ( device == NULL )
    {
        QString defaultTemplate ( "<tr><td bgcolor=\"#f0f0ff\">%1</td></tr>" );
        details->setText ( infoTemplate.arg ( defaultTemplate.arg ( "<i>" + tr ( "No Drive Selected" ) + "</i>" ) ) );
        return;
    }

    /*---get info about the hard disk using libparted---*/
    qtp_DriveInfo driveinfo = QP_LibParted::device_info ( device->shortname() );

    QString items;

    QString itemTemplate ( "<tr><th bgcolor=\"#c8c8ff\">%1</th>"
                           "<td align=\"center\" width=60% bgcolor=\"#f0f0ff\">%2</td></tr>" );

    items += itemTemplate.arg ( tr ( "Device:" ) ).arg ( driveinfo.device );

    items += itemTemplate.arg ( tr ( "Model:" ) ).arg ( driveinfo.model );

    items += itemTemplate.arg ( tr ( "Capacity (Mb):" ) ).arg ( driveinfo.mb_capacity );

    items += itemTemplate.arg ( tr ( "Length sectors:" ) ).arg ( driveinfo.length_sectors );

    if ( !device->canUpdateGeometry() )
        items += itemTemplate.arg ( tr ( "Status: " ) ).arg ( tr ( "readonly." ) );
    else if ( device->isBusy() )
        items += itemTemplate.arg ( tr ( "Status: " ) ).arg ( tr ( "busy." ) );
    else
        items += itemTemplate.arg ( tr ( "Status: " ) ).arg ( tr ( "available." ) );

    details->setText ( infoTemplate.arg ( items ) );
}

QActionGroup *QP_NavView::agDevices()
{
    return drivelist->agDevices();
}

QP_Device *QP_NavView::selDevice()
{
    return drivelist->selDevice();
}
