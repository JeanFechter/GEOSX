/*
 * DataObjectManager.cpp
 *
 *  Created on: Nov 21, 2014
 *      Author: rrsettgast
 */

#include "../intrinsic/DataObjectManager.hpp"

namespace geosx
{

DataObjectManager::DataObjectManager( std::string const & name ):
m_size(0),
m_name(name),
m_path(),
m_keyLookup(),
m_dataObjects(),
m_parent(nullptr),
m_subObjectManagers()
{}

DataObjectManager::~DataObjectManager()
{
  // TODO Auto-generated destructor stub
}

//DataObjectManager::DataObjectManager( DataObjectManager const & source ):
//    m_size( source.m_size ),
//    m_name( source.m_name ),
//    m_path( source.m_path ),
//    m_keyLookup( source.m_keyLookup ),
//    m_dataObjects( source.m_dataObjects ),
//    m_parent( source.m_parent )
//{}

DataObjectManager::DataObjectManager( DataObjectManager&& source ):
    m_size( std::move(source.m_size) ),
    m_name( std::move(source.m_name) ),
    m_path( std::move(source.m_path) ),
    m_keyLookup( std::move(source.m_keyLookup) ),
    m_dataObjects( std::move(source.m_dataObjects) ),
    m_parent( std::move(source.m_parent) )
{}


void DataObjectManager::resize( const std::size_t newsize )
{

  m_size = newsize;
}



} /* namespace ODS */