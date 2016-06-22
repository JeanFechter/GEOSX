/**
 * @file DataObjectManager.h
 * @date created on Nov 21, 2014
 * @author Randolph R. Settgast
 */


#ifndef DATAOBJECTMANAGER_H_
#define DATAOBJECTMANAGER_H_

#include "DataObject.hpp"
//#include "include/common.h"
#include <iostream>
//#include "CodingUtilities/ANSTexception.hpp"

/**
 * namespace to encapsulate functions in simulation tools
 */
namespace geosx
{

/**
 * @author Randolph R. Settgast
 *
 * class that encapsulates and manages a collection of DataObjects. Can be considered a "node" in a
 * hierarchy of managers that represent physical groupings of data.
 *
 */
class DataObjectManager
{
public:
  /**
   * @name constructors, destructor, copy, move, assignments
   */
  ///@{

  /**
   * @author Randolph R. Settgast
   * @param name the name of this object manager
   */
  explicit DataObjectManager( const std::string& name );

  /**
   *
   */
  virtual ~DataObjectManager();

  DataObjectManager( DataObjectManager&& other );


  DataObjectManager() = delete;
  DataObjectManager( const DataObjectManager& ) = delete;
  DataObjectManager& operator=(const DataObjectManager&) = delete;
  DataObjectManager& operator=(DataObjectManager&&) = delete;

  ///@}



  virtual const std::type_info& get_typeid() const
  {
    return typeid(*this);
  }


  template< typename T >
  T& RegisterChildDataObjectManager( const std::string& name );

  template< typename T >
  T& GetChildDataObjectManager( const std::string& name )
  {
    return *(m_subObjectManagers.at(name));
  }

  template< typename T >
  std::size_t RegisterDataObject( const std::string& name );


  std::size_t RegisterDataObject( const std::string& name, const rtTypes::TypeIDs& type )
  {
    return rtTypes::ApplyTypeLambda( type,
                                     [this, &name]( auto a ) -> size_t
                                     {
                                       return this->RegisterDataObject<decltype(a)>(name);
                                     } );
  }


  //***********************************************************************************************

  template< typename T >
  const T& GetDataObject( const std::size_t index ) const
  {
    return m_dataObjects[index]->getObject<T>();
  }
  template< typename T >
  T& GetDataObject( const std::size_t index )
  {
    return const_cast<T&>( const_cast<const DataObjectManager*>(this)->GetDataObject<T>( index ) );
  }


  template< typename T >
  const DataObject<T>& GetDataObject( const std::string& name ) const
  {
#if 1
    auto index = m_keyLookup.at(name);
    return m_dataObjects[index]->getObject<T>();
#else
    auto iter = m_keyLookup.find(name);
    const T* rval = nullptr;
    if( iter==m_keyLookup.end() )
    {
//      throw ANSTexception( LOCATION );
      throw std::exception();
    }
    else
    {
      std::size_t index = iter->second;
      rval = &(m_dataObjects[index]->getObjectData<T>());
    }
    return *rval;
#endif
  }
  template< typename T >
  DataObject<T>& GetDataObject( const std::string& name )
  { return const_cast<DataObject<T>&>( const_cast<const DataObjectManager*>(this)->GetDataObject<T>( name ) ); }








  template< typename T >
  const typename DataObject<T>::rtype GetDataObjectData( const std::size_t index ) const
  { return m_dataObjects[index]->getObjectData<T>(); }

  template< typename T >
  typename DataObject<T>::rtype GetDataObjectData( const std::size_t index )
  { return const_cast<T&>( const_cast<const DataObjectManager*>(this)->GetDataObjectData<T>( index ) ); }



  template< typename T >
  typename DataObject<T>::const_rtype GetDataObjectData( const std::string& name ) const
  {
    auto index = m_keyLookup.at(name);
    return m_dataObjects[index]->getObjectData<T>();
  }
  template< typename T >
  typename DataObject<T>::rtype GetDataObjectData( const std::string& name )
  { return const_cast<typename DataObject<T>::rtype>( const_cast<const DataObjectManager*>(this)->GetDataObjectData<T>( name ) ); }


  template< typename T >
  T& GetDataObjectManager( const std::string& name )
  {
    return *(m_subObjectManagers.at(name));
  }

  void resize( const std::size_t newsize );

  std::size_t size() const { return m_size; }

private:
  std::size_t m_size;
  std::string m_name{"name not set"};
  std::string m_path{"path not set"};
  std::unordered_map<std::string,std::size_t> m_keyLookup;
  std::vector< std::unique_ptr<DataObjectBase> > m_dataObjects;

  DataObjectManager* m_parent = nullptr;
  std::unordered_map< std::string, std::unique_ptr<DataObjectManager> > m_subObjectManagers;


};




template< typename T >
std::size_t DataObjectManager::RegisterDataObject( const std::string& name )
{
  std::size_t key = -1;
  auto iterKeyLookup = m_keyLookup.find(name);

  // if the key was not found, make DataObject<T> and insert
  if( iterKeyLookup == m_keyLookup.end() )
  {
    m_dataObjects.push_back( std::move( DataObjectBase::Factory<T>(name) ) );
    key = m_dataObjects.size() - 1;
    m_keyLookup.insert( std::make_pair(name,key) );
  }
  // if key was found, make sure that they are the same type
  else
  {
    key = m_keyLookup.at(name);
    auto& basePtr = m_dataObjects[key];
    if( typeid(T) != basePtr->get_typeid() )
    {
      std::cout<<LOCATION<<std::endl;
      throw std::exception();
    }
  }
  return key;
}

template< typename T >
T& DataObjectManager::RegisterChildDataObjectManager( const std::string& name )
{
  auto iterKeyLookup = m_subObjectManagers.find(name);

  // if the key was not found, make DataObject<T> and insert
  if( iterKeyLookup == m_subObjectManagers.end() )
  {
    auto insertResult = m_subObjectManagers.insert( std::make_pair(name,std::move( std::unique_ptr< T >( new T( name ) ) ) ) );

    if( !insertResult.second )
    {
      std::cout<<LOCATION<<std::endl;
      throw std::exception();
    }
    iterKeyLookup = insertResult.first;
  }
  // if key was found, make sure that they are the same type
  else
  {

    if( typeid(T) != iterKeyLookup->second->get_typeid() )
    {
      std::cout<<LOCATION<<std::endl;
      throw std::exception();
    }
  }
  return *(iterKeyLookup->second);
}

} /* namespace ODS */

#endif /* DATAOBJECTMANAGER_H_ */