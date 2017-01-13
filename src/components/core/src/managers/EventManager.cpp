/*
 * EventManager.cpp
 *
 *  Created on: Oct 5, 2016
 *      Author: sherman
 */

#include "EventManager.hpp"

#include "DocumentationNode.hpp"
#include "pugixml/src/pugixml.hpp"

namespace geosx
{

using namespace dataRepository;
using namespace cxx_utilities;

EventManager::EventManager( std::string const & name,
                            ManagedGroup * const parent ):
  ManagedGroup( name, parent)
{
}

EventManager::~EventManager()
{
}

void EventManager::FillDocumentationNode( dataRepository::ManagedGroup * const )
{
  cxx_utilities::DocumentationNode * const docNode = this->getDocumentationNode();
  
  docNode->setSchemaType("Node");
  docNode->setShortDescription("Contains the set of solver applications");
}


void EventManager::ReadXML( pugi::xml_node const & problemNode )
{
  pugi::xml_node topLevelNode = problemNode.child("SolverApplications");
  if (topLevelNode == NULL)
  {
    throw std::invalid_argument("SolverApplications block not present in input xml file!");
  }
  else
  {
    // Allow other event types here?
    for (pugi::xml_node applicationNode=topLevelNode.first_child(); applicationNode; applicationNode=applicationNode.next_sibling())
    {
      std::string applicationName = applicationNode.attribute("name").value();
      SolverApplication& newApplication = RegisterGroup<SolverApplication>(applicationName);
      newApplication.SetDocumentationNodes(this);
      newApplication.RegisterDocumentationNodes();
      newApplication.ReadXML(applicationNode);
    }
  }
}


void EventManager::CheckEventTiming()
{
  cxx_utilities::DocumentationNode * const docNode = getDocumentationNode();

  for (std::map<std::string,DocumentationNode>::iterator eit=docNode->m_child.begin(); eit!=docNode->m_child.end(); ++eit)
  {
    dataRepository::ManagedGroup& applicationA = GetGroup(eit->first);
    ViewWrapper<real64>::rtype endTime = applicationA.getData<real64>(keys::endTime);

    if (++eit != docNode->m_child.end())
    {
      dataRepository::ManagedGroup& applicationB = GetGroup(eit->first);
      ViewWrapper<real64>::rtype beginTime = applicationB.getData<real64>(keys::beginTime);

      if (fabs(*(beginTime) - *(endTime)) > 1e-6)
      {
        std::cout << "Error in solver application times: " << eit->first << std::endl;
        throw std::invalid_argument("Solver application times must be contiguous!");
      }

      --eit;
    }
  }
}




SolverApplication::SolverApplication( std::string const & name,
                            ManagedGroup * const parent ):
  ManagedGroup( name, parent)
{
}

SolverApplication::~SolverApplication()
{
}

void SolverApplication::FillDocumentationNode( dataRepository::ManagedGroup * const )
{
  cxx_utilities::DocumentationNode * const docNode = this->getDocumentationNode();
  
  docNode->setName(this->CatalogName());
  docNode->setSchemaType("Node");
  docNode->setShortDescription("Describes the timing of the solver application");

  docNode->AllocateChildNode( keys::beginTime,
                              keys::beginTime,
                              -1,
                              "real64",
                              "real64",
                              "application start time",
                              "application start time",
                              "0.0",
                              "",
                              1,
                              0 );

  docNode->AllocateChildNode( keys::endTime,
                              keys::endTime,
                              -1,
                              "real64",
                              "real64",
                              "application endTime",
                              "application endTime",
                              "1.0e9",
                              "",
                              1,
                              0 );

  docNode->AllocateChildNode( keys::dt,
                              keys::dt,
                              -1,
                              "real64",
                              "real64",
                              "application dt",
                              "application dt",
                              "-1.0",
                              "",
                              1,
                              0 );

  docNode->AllocateChildNode( keys::solvers,
                              keys::solvers,
                              -1,
                              "string_array",
                              "string_array",
                              "application solvers",
                              "application solvers",
                              "",
                              "",
                              1,
                              0 );

}


} /* namespace geosx */