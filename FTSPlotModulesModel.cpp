/* FTSPlot - fast time series dataset plotter
   Copyright (C) 2013  Michael Riss <Michael.Riss@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA */


#include <FTSPlotModulesModel.h>
#include <FTSPlotWidget.h>

#include <QDebug>

using namespace FTSPlot;

FTSPlotModulesModel::FTSPlotModulesModel ( FTSPlotWidget* upper ) : QAbstractTableModel ( upper )
{
    dataObject = upper;
}


int FTSPlotModulesModel::rowCount ( const QModelIndex& parent ) const
{
    return dataObject->modules.size();
}

int FTSPlotModulesModel::columnCount ( const QModelIndex& parent ) const
{
    return 5;
}

QVariant FTSPlotModulesModel::data ( const QModelIndex& index, int role ) const
{
    if ( ( index.row() >= dataObject->modules.size() ) || ( index.column() >= 5 ) )
    {
        return QVariant();
    }

    // access data structure and return value
    if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        if ( dataObject->modules[index.row() ].visible )
        {
            return QVariant ( Qt::Checked );
        }
        else
        {
            return QVariant ( Qt::Unchecked );
        }
    }
    if ( index.column() == 1 && role == Qt::CheckStateRole )
    {
        if ( dataObject->modules[index.row() ].module->GUIvisible() )
        {
            return QVariant ( Qt::Checked );
        }
        else
        {
            return QVariant ( Qt::Unchecked );
        }
    }
    if ( index.column() == 2 && role == Qt::CheckStateRole )
    {
        if ( dataObject->activeInputModule == dataObject->modules[index.row() ].module )
        {
            return QVariant ( Qt::Checked );
        }
        else
        {
            return QVariant ( Qt::Unchecked );
        }
    }
    if ( index.column() == 3 && ( role == Qt::DisplayRole || role == Qt::EditRole ) )
    {
        return QVariant ( dataObject->modules[index.row() ].name );
    }
    if ( index.column() == 3 && role == Qt::ToolTipRole )
    {
        return QVariant ( dataObject->modules[index.row() ].module->getPath() );
    }
    if ( index.column() == 4 && role == Qt::DecorationRole )
    {
        return QVariant ( dataObject->modules[index.row() ].color );
    }

    // else just return the empty value
    return QVariant();
}

bool FTSPlotModulesModel::setData ( const QModelIndex& index, const QVariant& value, int role )
{
    if ( ( index.row() >= dataObject->modules.size() ) || ( index.column() >= 5 ) )
    {
        return false;
    }

    if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        dataObject->modules[index.row() ].visible = value.toBool();
        emit dataChanged ( index, index );
        dataObject->updateVizList();
        return true;
    }
    if ( index.column() == 1 && role == Qt::CheckStateRole )
    {
        if ( value.toBool() )
        {
            dataObject->modules[index.row() ].module->showGUI();
        }
        else
        {
            dataObject->modules[index.row() ].module->hideGUI();
        }
        emit dataChanged ( index, index );
        return true;
    }
    if ( index.column() == 2 && role == Qt::CheckStateRole )
    {
        if ( value.toBool() )
        {
            dataObject->activeInputModule = dataObject->modules[index.row() ].module;
            emit dataChanged ( QAbstractItemModel::createIndex ( 0, 2 ), QAbstractItemModel::createIndex ( dataObject->modules.size()-1, 2 ) );
            return true;
        }
        else
        {
            if ( dataObject->activeInputModule == dataObject->modules[index.row() ].module )
            {
                dataObject->activeInputModule = NULL;
                emit dataChanged ( index, index );
                return true;
            }
            else
            {
                // we cannot deactivate the current module if it's not activated
                return false;
            }
        }
    }
    if ( index.column() == 3 && role == Qt::EditRole )
    {
        dataObject->modules[index.row() ].name = value.toString();
        emit dataChanged ( index, index );
        return true;
    }
    if ( index.column() == 4 && role == Qt::DecorationRole )
    {
        dataObject->modules[index.row() ].color = value.value<QColor>();
        dataObject->modules[index.row() ].module->setColor ( value.value<QColor>() );
        dataObject->update();
        emit dataChanged ( index, index );
        return true;
    }
    return false;
}

QVariant FTSPlotModulesModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
    {
        switch ( section )
        {
        case 0:
            return QVariant ( QString ( "Graph" ) );
            break;
        case 1:
            return QVariant ( QString ( "GUI" ) );
            break;
        case 2:
            return QVariant ( QString ( "Edit active" ) );
            break;
        case 3:
            return QVariant ( QString ( "Name" ) );
            break;
        case 4:
            return QVariant ( QString ( "Color" ) );
            break;
        default:
            qDebug() << "QVariant SimpleViewModulesModel::headerData() should never get sections >= 4!";
            break;
        }

    }
    return QVariant();
}

Qt::ItemFlags FTSPlotModulesModel::flags ( const QModelIndex& index ) const
{
    switch ( index.column() )
    {
    case 0:
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        break;
    case 1:
        if ( dataObject->modules[index.row() ].module->hasGUI() )
        {
            return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        }
        else
        {
            return ( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
        }
        break;
    case 2:
        if ( dataObject->modules[index.row() ].editable )
        {
            return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
        }
        else
        {
            return ( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable );
        }
        break;
    case 3:
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable );
        break;
    case 4:
        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable );
        break;
    default:
        qDebug() << "Qt::ItemFlags SimpleViewModulesModel::flags() should never get columns >= 4!";
        break;
    }

    return Qt::NoItemFlags;
}

bool FTSPlotModulesModel::insertRows ( int position, int rows, const QModelIndex& index )
{
    if ( position >= dataObject->modules.size() )
    {
        return false;
    }

    beginInsertRows ( QModelIndex(), position, position+rows-1 );

    vizModule tmp;
    tmp.visible = false;
    tmp.editable = false;
    for ( int i = 0; i < rows; i++ )
    {
        dataObject->modules.insert ( position, tmp );
    }

    endInsertRows();

    return true;
}

bool FTSPlotModulesModel::removeRows ( int position, int rows, const QModelIndex& index )
{
    if ( position >= dataObject->modules.size() )
    {
        return false;
    }
    if ( position + rows > dataObject->modules.size() )
    {
        return false;
    }

    beginRemoveRows ( QModelIndex(), position, position+rows-1 );

    for ( int i = 0; i < rows; i++ )
    {
        if( dataObject->activeInputModule == dataObject->modules[position].module )
        {
            dataObject->activeInputModule = NULL;
        }
        dataObject->modules.removeAt ( position );
    }

    endRemoveRows();

    return true;
}

void FTSPlotModulesModel::append ( vizModule& mod )
{
    beginInsertRows ( QModelIndex(), dataObject->modules.size(), dataObject->modules.size() );
    dataObject->modules.append ( mod );
    endInsertRows();
}

void FTSPlotModulesModel::prepend ( vizModule& mod )
{
    beginInsertRows ( QModelIndex(), 0, 0 );
    dataObject->modules.prepend ( mod );
    endInsertRows();
}

void FTSPlotModulesModel::GUIupdate()
{
    emit dataChanged ( QAbstractItemModel::createIndex ( 0, 1 ), QAbstractItemModel::createIndex ( dataObject->modules.size()-1, 1 ) );
}

void FTSPlotModulesModel::swapRows ( int first, int second )
{
    if( first >= 0 && first < dataObject->modules.size() && 
        second >= 0 && second < dataObject->modules.size() )
    {
        dataObject->modules.swap( first, second );
        dataObject->updateVizList();
        emit dataChanged ( QAbstractItemModel::createIndex ( first, 0 ), QAbstractItemModel::createIndex ( first, 4 ) );
        emit dataChanged ( QAbstractItemModel::createIndex ( second, 0 ), QAbstractItemModel::createIndex ( second, 4 ) );
    }
}

void FTSPlotModulesModel::deleteModule ( int idx )
{
    // take module out of module list
    vizModule tmp = dataObject->modules[idx];
    removeRows ( idx, 1, QModelIndex() );
    // purge all instances of the module in temporary lists
    dataObject->useList.removeAll( tmp.module );
    dataObject->reqList.removeAll( tmp.module );
    dataObject->vizList.removeAll( tmp.module );
    dataObject->updateList.removeAll( tmp.module );
    dataObject->reqSet.remove( tmp.module );
    // delete module
    delete( tmp.module );
    dataObject->updateVizList(); 
}
