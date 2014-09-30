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

#include <qapplication.h>
#include <qmessagebox.h>
#include "qp_filesystem.h"
#include "qp_actlist.h"
#include "qp_debug.h"
#include "statistics.h"

/*---type (move+resize), num, start, end---*/
QP_ActListItem::QP_ActListItem ( QTParted::actType action, int num,
                                 PedSector start, PedSector end,
                                 PedGeometry geom, PedPartitionType part_type )
        : _action ( action ), _num ( num ), _start ( start ),
        _end ( end ), _geom ( geom ), _part_type ( part_type )
{
    showDebug ( "%s", "actlistitem::actlistitem, move/resize\n" );
}

/*---type, num---*/
QP_ActListItem::QP_ActListItem ( QTParted::actType action, int num )
        : _action ( action ), _num ( num )
{
    showDebug ( "%s", "actlistitem::actlistitem, rm\n" );
}

QP_ActListItem::QP_ActListItem ( QTParted::actType action, int num, bool status )
        : _action ( action ), _num ( num ), _status ( status )
{
    showDebug ( "%s", "actlistitem::actlistitem, active\n" );
}

/*---for format! :)---*/
QP_ActListItem::QP_ActListItem ( QTParted::actType action,
                                 int num,
                                 QP_FileSystemSpec *fsspec,
                                 QString label,
                                 PedGeometry geom,
                                 PedPartitionType part_type )
        : _action ( action ), _num ( num ), _fsspec ( fsspec )
        , _label ( label ), _geom ( geom ), _part_type ( part_type )
{
    showDebug ( "%s", "actlistitem::actlistitem, mkfs\n" );
}

/*---type, logical/extended, start, end, typoFS, label---*/
QP_ActListItem::QP_ActListItem ( QTParted::actType action,
                                 QTParted::partType type,
                                 PedSector start,
                                 PedSector end,
                                 QP_FileSystemSpec *fsspec,
                                 QString label,
                                 PedGeometry geom,
                                 PedPartitionType part_type )
        : _action ( action ), _type ( type ), _start ( start )
        , _end ( end ), _fsspec ( fsspec ), _label ( label )
        , _geom ( geom ), _part_type ( part_type )
{
    showDebug ( "%s", "actlistitem::actlistitem, mkpartfs\n" );
}

QP_ActionList::QP_ActionList ( QP_LibParted *libparted ) : _libparted ( libparted )
{
    showDebug ( "%s", "actionlist::actionlist\n" );

    PedDisk *disk;

    /*---save of the original device state---*/
    disk = ped_disk_new ( _libparted->dev );

    if ( !disk )
    {
        showDebug ( "%s", "actionlist::actionlist, ped_disk_new ko\n" );

        QString label = QString ( tr ( "Critical error during ped_disk_new!" ) );
        QMessageBox::information ( NULL, "QtParted", label );

        return ;
    }

    listdisk.append ( disk );

    /*---make a backup of the disk (we will use this)---*/
    _disk = ped_disk_duplicate ( disk );

    if ( !_disk )
    {
        showDebug ( "%s", "actionlist::actionlist, ped_disk_duplicate ko\n" );
        ped_disk_destroy ( disk );
        listdisk.removeLast();
        disk = NULL;

        QString label = QString ( tr ( "Critical error during ped_disk_duplicate!" ) );
        QMessageBox::information ( NULL, "QtParted", label );

        return ;
    }

    /*---make the partlist of the disk---*/
    scan_partitions();
}

QP_ActionList::~QP_ActionList()
{
    showDebug ( "%s", "actionlist::~actionlist\n" );

    actlist.clear();
    orig_logilist.clear();
    orig_partlist.clear();

}

void QP_ActionList::update_listpartitions()
{
    showDebug ( "%s", "actionlist::update_listpartitions\n" );
    /*---initialize active partition---*/
    _partActive = NULL;

    logilist.clear();
    partlist.clear();

    _libparted->has_extended = false;


    /*---look if exist a partition in orig_list that match with the partitions into disk device---*/
    PedConstraint* constraint = NULL;
    PedPartition*	part = NULL;

    for ( part = ped_disk_next_partition ( disk(), NULL ); part;
            part = ped_disk_next_partition ( disk(), part ) )
    {

        if ( part->type & PED_PARTITION_METADATA )
            continue;

        QP_PartInfo *found = NULL;

        /*---if not freespace...*/
        if ( ! ( part->type & PED_PARTITION_FREESPACE ) )
        {
            /*---loop for all partition of the disk---*/
            foreach ( QP_PartInfo *p, orig_partlist )
            {
                if ( p->type != QTParted::extended )
                {
                    if ( ( p->_geometry.start == part->geom.start )
                            && ( p->_geometry.end == part->geom.end )
                            && ( p->_geometry.length == part->geom.length ) )
                    {
                        found = p;
                    }
                }
                else
                {
                    /*---loop for every logical partitions---*/
                    foreach ( QP_PartInfo* logi, orig_logilist )
                    {
                        if ( ( logi->_geometry.start == part->geom.start )
                                && ( logi->_geometry.end == part->geom.end )
                                && ( logi->_geometry.length == part->geom.length ) )
                        {
                            found = logi;
                        }
                    }
                }
            }
        }

        showDebug ( "%s", "actionlist::update_listpartitions, looked if partition was in orig_logilist\n" );

        QP_PartInfo *partinfo;

        if ( found )
        {
            showDebug ( "%s", "actionlist::update_listpartitions, partition matched!\n" );
            partinfo = new QP_PartInfo();
            partinfo->start = found->start;
            partinfo->end = found->end;
            partinfo->_geometry = found->_geometry;
            partinfo->setDevice ( found->device() );
            partinfo->min_size = found->min_size;
            partinfo->_label = found->_label;
            partinfo->_free = found->_free;
            partinfo->_unknown = found->_unknown;
            partinfo->_libparted = found->_libparted;
            partinfo->_active = found->_active;
            partinfo->_canBeActive = found->_canBeActive;
            partinfo->_canBeHidden = found->_canBeHidden;
            partinfo->_virtual = found->_virtual;
            partinfo->fsspec = found->fsspec;
        }
        else
        {
            showDebug ( "%s", "actionlist::update_listpartitions, partition missmatched!\n" );
            partinfo = new QP_PartInfo();
            partinfo->start = part->geom.start;
            partinfo->end = part->geom.end;
            partinfo->_geometry = part->geom;
            partinfo->setDevice ( _libparted->_qpdevice );
            partinfo->min_size = -1;
            partinfo->_label = QString::null;
            partinfo->_free = _libparted->filesystem->free();
            partinfo->_unknown = _libparted->filesystem->unknown();
            partinfo->_libparted = _libparted;
            partinfo->_active = false;
            partinfo->_canBeActive = false;
            partinfo->_canBeHidden = false;
            partinfo->_virtual = false;


            const char *part_filesystem = part->fs_type ? part->fs_type->name : "";

            if ( part->type & PED_PARTITION_FREESPACE )
            {
                partinfo->fsspec = _libparted->filesystem->free();
            }
            else
            {
                partinfo->fsspec = _libparted->filesystem->nameToFSSpec ( part_filesystem ); //fat16, fat32 etc etc
            }
        }

        //'cause the number can change use ALWAYS the right PedPartition number!
        partinfo->num = part->num;

        if ( part->type & PED_PARTITION_LOGICAL )
            partinfo->type = QTParted::logical;
        else if ( part->type & PED_PARTITION_EXTENDED )
        {
            partinfo->type = QTParted::extended;
            _libparted->has_extended = true;
        }
        else partinfo->type = QTParted::primary;


        showDebug ( "%s", "actionlist::update_listpartitions, get partitions flags\n" );

        //---get how much the partition can grow at left and at right
        //---get also if the partiton is "active"
        //---of course do all this stuff only if this is not freespace ;)
        //FIXME: t_start/t_end if the partition is free? is right?
        if ( !partinfo->isFree() )
        {
            constraint = ped_constraint_any ( _libparted->dev );
            PedGeometry *geometry = ped_disk_get_max_partition_geometry ( disk(), part, constraint );

            if ( geometry != NULL )
            {
                partinfo->t_start = geometry->start;
                partinfo->t_end = geometry->end;
            }
            else
            {
                partinfo->t_start = -1;
                partinfo->t_end = -1;
            }

            /*---get if the partition is active---*/
            partition_get_flags ( partinfo, part );
        }

        showDebug ( "%s", "actionlist::update_listpartitions, get partition infos\n" );

        /*---compare if the partition has not committed... and if so update
         *   some field (ie filesystem, and active flag, to reflect the change that
         *   use made on listchart but that are not saved on the disk!---*/
        get_partinfo ( partinfo, part );

        /*---if this is a virtual partition do not show how space fill the device---*/

        if ( partinfo->isVirtual() )
        {
            partinfo->min_size = -1;
        }

        showDebug ( "%s", "actionlist::update_listpartitions, append the partition to partlist\n" );

        if ( partinfo->type == QTParted::logical )
            logilist.append ( partinfo );
        else partlist.append ( partinfo );

    }

}

void QP_ActionList::scan_partitions()
{
    showDebug ( "%s", "actionlist::scan_partitions\n" );

    /*---initialize active partition---*/
    _partActive = NULL;

    orig_logilist.clear();
    orig_partlist.clear();

    PedConstraint *constraint = NULL;
    PedPartition *part = NULL;

    for ( part = ped_disk_next_partition ( disk(), NULL ); part;
            part = ped_disk_next_partition ( disk(), part ) )
    {

        if ( part->type & PED_PARTITION_METADATA )
            continue;

        showDebug ( "%s", "actionlist::scan_partitions, create a new partinfo\n" );

        QP_PartInfo *partinfo = new QP_PartInfo();

        partinfo->start = part->geom.start;

        partinfo->end = part->geom.end;

        partinfo->_geometry = part->geom;

        partinfo->num = part->num;

        partinfo->setDevice ( _libparted->_qpdevice );

        partinfo->min_size = -1;

        partinfo->_label = QString::null;

        partinfo->_free = _libparted->filesystem->free();

        partinfo->_unknown = _libparted->filesystem->unknown();

        partinfo->_libparted = _libparted;

        partinfo->_active = false;

        partinfo->_canBeActive = false;

        partinfo->_canBeHidden = false;

        partinfo->_virtual = false;

        if ( part->type & PED_PARTITION_LOGICAL )
            partinfo->type = QTParted::logical;
        else if ( part->type & PED_PARTITION_EXTENDED )
        {
            partinfo->type = QTParted::extended;
        }
        else partinfo->type = QTParted::primary;

        const char *part_filesystem = part->fs_type ? part->fs_type->name : "";

        if ( part->type & PED_PARTITION_FREESPACE )
        {
            partinfo->fsspec = _libparted->filesystem->free();
            partinfo->t_start = -1;
            partinfo->t_end = -1;
        }
        else
        {
            showDebug ( "%s", "actionlist::scan_partitions, get partinfo flags\n" );
            /*---get if the partition is active---*/
            partition_get_flags ( partinfo, part );

            showDebug ( "%s", "actionlist::scan_partitions, get partinfo fsspec\n" );
            partinfo->fsspec = _libparted->filesystem->nameToFSSpec ( part_filesystem ); //fat16, fat32 etc etc

            //---get how much the partition can grow at left and at right, but only if the partition is not free space!
            constraint = ped_constraint_any ( _libparted->dev );
            PedGeometry *geometry =
                ped_disk_get_max_partition_geometry ( disk(), part, constraint );

            if ( geometry != NULL )
            {
                partinfo->t_start = geometry->start;
                partinfo->t_end = geometry->end;
            }
            else
            {
                partinfo->t_start = -1;
                partinfo->t_end = -1;
            }
        }

        showDebug ( "%s", "actionlist::scan_partitions, append partinfo to orig_partlist\n" );

        if ( partinfo->type == QTParted::logical )
            orig_logilist.append ( partinfo );
        else orig_partlist.append ( partinfo );
    }



    /*---count how partition are in the disk---*/
    int totPart = orig_partlist.count() + orig_logilist.count();

    showDebug ( "%s", "actionlist::scan_partitions, loop into orig_partlist\n" );

    /*---loop for all partition of the disk---*/
    int i = 0;

    foreach ( QP_PartInfo *p, orig_partlist )
    {
        if ( p->type != QTParted::extended )
        {
            i++;

            if ( !p->isFree() )
            {
                /*---emit a signal for update the progressbar---*/
                _libparted->_message = QString ( tr ( "Getting info about partition %1." ) )
                                       .arg ( p->partname() );
                _libparted->emitSigTimer ( i*100 / totPart, _libparted->message(), QString::null );

                /*---get info about this primary partition---*/
                part = ped_disk_get_partition ( disk(), p->num );

                if ( part ) get_partfilesystem_info ( part, p );
                else showDebug ( "%s", "actionlist::scan_partitions, get_partfilesystem_info ko\n" );

                /*---get the label of this primary partition---*/
                if ( part ) get_partfilesystem_label ( part, p );
                else showDebug ( "%s", "actionlist::scan_partitions, get_partfilesystem_label ko\n" );
            }
        }
        else
        {
            foreach ( QP_PartInfo *logi, orig_logilist )
            {
                /*---loop for every logical partitions---*/
                i++;

                if ( !logi->isFree() )
                {
                    /*---emit a signal for update the progressbar---*/
                    _libparted->_message = QString ( tr ( "Getting info about partition %1." ) )
                                           .arg ( logi->partname() );
                    _libparted->emitSigTimer ( i*100 / totPart, _libparted->message(), QString::null );

                    /*---get info about this logical partition---*/
                    part = ped_disk_get_partition ( disk(), logi->num );

                    if ( part ) get_partfilesystem_info ( part, logi );
                    else showDebug ( "%s", "actionlist::scan_partitions, get_partfilesystem_info ko\n" );

                    /*---get the label of this logical partition---*/
                    if ( part ) get_partfilesystem_label ( part, logi );
                    else showDebug ( "%s", "actionlist::scan_partitions, get_partfilesystem_label ko\n" );
                }
            }
        }
    }

    _libparted->emitSigTimer ( 100, _libparted->message(), QString::null );
}

bool QP_ActionList::get_partfilesystem_info ( PedPartition *part, QP_PartInfo *partinfo )
{
    showDebug ( "%s", "actionlist::get_partfilesystem_info\n" );

    /*---if the partition is virtual (ie change not committed) we cannot determinate
     *   how much the filesystem is fill---*/

    if ( partinfo->_virtual )
    {
        partinfo->min_size = -1;
        return true;
    }

#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
    /*---is the filesystem supported by parted?---*/
    PedFileSystem *fs = ped_file_system_open ( &part->geom );
#else
    void *fs = 0;
#endif

    if ( !fs )
    {
        /*---exist a wrapper for min_size?---*/
        if ( partinfo->fswrap() && partinfo->fsspec->fswrap()->wrap_min_size )
        {
            /*---get the min_size from the wrapper---*/
            partinfo->min_size = partinfo->fsspec->fswrap()->min_size ( partinfo->partname() );

            if ( partinfo->min_size > ( partinfo->end - partinfo->start ) ) partinfo->min_size = partinfo->end - partinfo->start;
        }
        else
            /*---get the min_size from space_stats (that is a "df" wrapper)---*/
            partinfo->min_size = space_stats ( partinfo );

        return true;
    }

    /*---get the minimum filesystem size---*/
#ifdef USE_PARTED2_FS_SUPPORT // Filesystem support was removed from parted 3.x
    PedConstraint *resize_constraint = ped_file_system_get_resize_constraint ( fs );

    if ( resize_constraint )
    {
        partinfo->min_size = resize_constraint->min_size;
        ped_constraint_destroy ( resize_constraint );
    }

    ped_file_system_close ( fs );
#endif

    return true;
}

bool QP_ActionList::get_partfilesystem_label ( PedPartition *part, QP_PartInfo *partinfo )
{
    if ( partinfo->_virtual )
        return true;

    partinfo->_label = QP_FSWrap::get_label ( part, partinfo->fsspec->name() );

    return true;
}

void QP_ActionList::ins_resize ( int num,
                                 PedSector start,
                                 PedSector end,
                                 PedGeometry geom,
                                 PedPartitionType part_type )
{
    showDebug ( "%s", "actionlist::ins_resize\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::resize, num, start, end,
            geom, part_type );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_move ( int num,
                               PedSector start,
                               PedSector end,
                               PedGeometry geom,
                               PedPartitionType part_type )
{
    showDebug ( "%s", "actionlist::ins_move\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::move, num, start, end,
            geom, part_type );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_rm ( int num )
{
    showDebug ( "%s", "actionlist::ins_rm\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::rm, num );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_mkfs ( QP_FileSystemSpec *fsspec, int num, QString label, PedGeometry geom, PedPartitionType part_type )
{
    showDebug ( "%s", "actionlist::ins_mkfs\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::format, num, fsspec, label, geom, part_type );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_mkpart ( QTParted::partType type,
                                 PedSector start,
                                 PedSector end,
                                 QP_FileSystemSpec *fsspec,
                                 QString label,
                                 PedGeometry geom,
                                 PedPartitionType part_type )
{
    showDebug ( "%s", "actionlist::ins_mkpart\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::create, type,
            start, end, fsspec, label, geom, part_type );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_active ( int num, bool active )
{
    showDebug ( "%s", "actionlist::ins_active\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::active, num, active );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::ins_hidden ( int num, bool hidden )
{
    showDebug ( "%s", "actionlist::ins_hidden\n" );

    QP_ActListItem *actlistitem = new QP_ActListItem ( QTParted::hidden, num, hidden );
    actlist.append ( actlistitem );

    ins_newdisk();
}

void QP_ActionList::get_partinfo ( QP_PartInfo *partinfo, PedPartition *part )
{
    showDebug ( "%s", "actionlist::get_partinfo\n" );

    /*---loop for every action saved---*/
    foreach(QP_ActListItem *pl, actlist) {
        if ( ( pl->_action == QTParted::create )
                || ( pl->_action == QTParted::resize )
                || ( pl->_action == QTParted::move )
                || ( pl->_action == QTParted::format ) )
        {
            if ( ( part->geom.start == pl->_geom.start )
                    && ( part->geom.end == pl->_geom.end )
                    && ( part->geom.length == pl->_geom.length )
                    && ( part->type == pl->_part_type ) )
            {
                partinfo->_virtual = true;

                if ( pl->_action == QTParted::create ) partinfo->fsspec = pl->_fsspec;

                if ( pl->_action == QTParted::format ) partinfo->fsspec = pl->_fsspec;
            }
        }
    }
}

bool QP_ActionList::canUndo()
{
    return ( listdisk.first() != listdisk.last() );
}

void QP_ActionList::undo()
{
    showDebug ( "%s", "actionlist::undo\n" );

    /*---destroy the state of the disk in this moment---*/
    ped_disk_destroy ( _disk );

    /*---undo last operation---*/
    PedDisk *disk = ( PedDisk * ) listdisk.last();
    ped_disk_destroy ( disk );
    listdisk.removeLast();

    /*---restore last state---*/
    _disk = ped_disk_duplicate ( ( PedDisk * ) listdisk.last() ); //FIXME: if !_disk :(

    if ( !_disk ) showDebug ( "%s", "actionlist::undo, ped_disk_duplicate ko\n" );

    /*---remove last operation---*/
    actlist.removeLast();

    /*---the state of the disk... is of course changed---*/
    emit sigDiskChanged();
}

void QP_ActionList::commit()
{
    showDebug ( "%s", "actionlist::commit\n" );

    //messageState, used to keep "error message" returned by libparted
    QString messageState = QString::null;

    /*---undo all disk state---*/

    while ( listdisk.first() != listdisk.last() )
    {
        PedDisk *disk = ( PedDisk * ) listdisk.last();
        ped_disk_destroy ( disk );
        listdisk.removeLast();
    }

    _disk = ped_disk_duplicate ( ( PedDisk * ) listdisk.first() );

    if ( !_disk ) showDebug ( "%s", "actionlist::commit, ped_disk_duplicate ko\n" );

    /*---commit the operations in "batch" mode---*/
    _libparted->setWrite ( true );

    bool rc = true;

    //counter of how much operations are done!
    int i = 0;

    int iTotAct = actlist.count() + 1;

    foreach(QP_ActListItem *pl, actlist) {
        showDebug ( "%s", "actionlist::commit, loop for commit\n" );

        //---mkpart commit---

        if ( pl->_action == QTParted::create )
        {
            showDebug ( "%s", "actionlist::commit, want to commit a create\n" );
            emit sigOperations ( tr ( "Creating partition." ), messageState, i++, iTotAct );

            if ( !_libparted->mkpartfs ( pl->_type, pl->_fsspec, pl->_start, pl->_end, pl->_label ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---rm commit---
        else if ( pl->_action == QTParted::rm )
        {
            showDebug ( "%s", "actionlist::commit, want to commit a rm\n" );
            emit sigOperations ( tr ( "Preparation for removing a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Removing a partition." ), messageState, i, iTotAct );

            if ( !_libparted->rm ( pl->_num ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---resize commit---
        else if ( pl->_action == QTParted::resize )
        {
            showDebug ( "%s", "actionlist::commit, want to commit a resize\n" );
            emit sigOperations ( tr ( "Preparation for resizing a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Resizing a partition." ), messageState, i, iTotAct );

            if ( !_libparted->resize ( pl->_num, pl->_start, pl->_end ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---move commit---
        else if ( pl->_action == QTParted::move )
        {
            showDebug ( "%s", "actionlist::commit, want to commit a move\n" );
            emit sigOperations ( tr ( "Preparation for moving a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Moving a partition." ), messageState, i, iTotAct );

            if ( !_libparted->move ( pl->_num, pl->_start, pl->_end ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---active commit---
        else if ( pl->_action == QTParted::active )
        {
            showDebug ( "%s", "actionlist::commit, want to commit an active\n" );
            emit sigOperations ( tr ( "Preparation for activating a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Activating a partition." ), messageState, i, iTotAct );

            if ( !_libparted->partition_set_flag_active ( pl->_num, pl->_status ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---active commit---
        else if ( pl->_action == QTParted::hidden )
        {
            showDebug ( "%s", "actionlist::commit, want to commit an hidden\n" );
            emit sigOperations ( tr ( "Preparation for hiding a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Hiding a partition." ), messageState, i, iTotAct );

            if ( !_libparted->partition_set_flag_hidden ( pl->_num, pl->_status ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        //---active commit---
        else if ( pl->_action == QTParted::format )
        {
            showDebug ( "%s", "actionlist::commit, want to commit a format\n" );
            emit sigOperations ( tr ( "Preparation for formatting a partition." ), messageState, i++, iTotAct );
            scan_partitions();
            _libparted->scan_orig_partitions();

            emit sigOperations ( tr ( "Formatting a partition." ), messageState, i, iTotAct );

            if ( !_libparted->mkfs ( pl->_num, pl->_fsspec, pl->_label ) )
            {
                messageState = _libparted->message();
                rc = false;
            }
        }

        /*---just update GUI---*/
        qApp->processEvents();

        if ( rc == false )
        {
            break;
        }
    }

    actlist.clear();

    /*---return in test mode---*/
    _libparted->setWrite ( false );

    /*---remove the disk used---*/
    ped_disk_destroy ( _disk );

    /*---remove also the original disk---*/
    ped_disk_destroy ( ( PedDisk * ) listdisk.first() );
    listdisk.clear();

    /*---prepare the list and the _disk---*/
    PedDisk *disk = ped_disk_new ( _libparted->dev ); //FIXME !disk

    if ( !disk ) showDebug ( "%s", "actionlist::commit, ped_disk_new ko\n" );

    listdisk.append ( disk );

    _disk = ped_disk_duplicate ( disk ); //FIXME !_disk

    if ( !_disk ) showDebug ( "%s", "actionlist::commit, ped_disk_duplicate ko\n" );

    emit sigOperations ( tr ( "Rescan of the disk." ), messageState, i, iTotAct );

    showDebug ( "%s", "actionlist::commit, call scan_partitions\n" );

    /*---make a new scan of the partitions---*/
    scan_partitions();

    emit sigOperations ( tr ( "All operations completed." ), messageState, iTotAct, iTotAct );

    /*---the state of the disk... is of course changed---*/
    emit sigDiskChanged();
}

PedDisk * QP_ActionList::disk()
{
    return _disk;
}

QP_PartInfo * QP_ActionList::partActive()
{
    return _partActive;
}

/*---this will get if the partition has active flag (ie is bootable)---*/
void QP_ActionList::partition_get_flags ( QP_PartInfo *partinfo, PedPartition *part )
{
    if ( ped_partition_is_flag_available ( part, PED_PARTITION_BOOT ) )
    {
        partinfo->_canBeActive = true;
        bool active = bool ( ped_partition_get_flag ( part, PED_PARTITION_BOOT ) );
        partinfo->_active = active;

        if ( active ) _partActive = partinfo;
    }

    if ( ped_partition_is_flag_available ( part, PED_PARTITION_HIDDEN ) )
    {
        partinfo->_canBeHidden = true;
        bool hidden = bool ( ped_partition_get_flag ( part, PED_PARTITION_HIDDEN ) );
        partinfo->_hidden = hidden;
    }
}

void QP_ActionList::ins_newdisk()
{
    showDebug ( "%s", "actionlist::ins_newdisk\n" );

    listdisk.append ( _disk );
    _disk = ped_disk_duplicate ( _disk ); //FIXME !_disk

    if ( !_disk ) showDebug ( "%s", "actionlist::ins_newdisk, ped_disk_duplicate ko\n" );

    emit sigDiskChanged();
}
