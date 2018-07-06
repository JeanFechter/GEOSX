/*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Copyright (c) 2018, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory
 *
 * LLNL-CODE-746361
 *
 * All rights reserved. See COPYRIGHT for details.
 *
 * This file is part of the GEOSX Simulation Framework.
 *
 * GEOSX is a free software; you can redistrubute it and/or modify it under
 * the terms of the GNU Lesser General Public Liscense (as published by the
 * Free Software Foundation) version 2.1 dated February 1999.
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

/**
 * @file SinglePhaseFlow_TPFA.hpp
 */

#ifndef SINGLE_PHASE_FLOW_TPFA_HPP_
#define SINGLE_PHASE_FLOW_TPFA_HPP_

#include "physicsSolvers/SolverBase.hpp"

class Epetra_FECrsGraph;

namespace geosx
{
namespace dataRepository
{
class ManagedGroup;
}
class BoundaryConditionBase;
class FiniteElementBase;
class DomainPartition;

/**
 * @class SinglePhaseFlow_TPFA
 *
 * class to perform a single phase, two-point flux approximation finite volume solve.
 */
class SinglePhaseFlow_TPFA : public SolverBase
{
public:
  /**
   * @brief main constructor for NodeManager Objects
   * @param name the name of this instantiation of NodeManager in the repository
   * @param parent the parent group of this instantiation of NodeManager
   */
  SinglePhaseFlow_TPFA( const std::string& name,
                        ManagedGroup * const parent );


  /// deleted default constructor
  SinglePhaseFlow_TPFA() = delete;

  /// deleted copy constructor
  SinglePhaseFlow_TPFA( SinglePhaseFlow_TPFA const & ) = delete;

  /// default move constructor
  SinglePhaseFlow_TPFA( SinglePhaseFlow_TPFA && ) = default;

  /// deleted assignment operator
  SinglePhaseFlow_TPFA & operator=( SinglePhaseFlow_TPFA const & ) = delete;

  /// deleted move operator
  SinglePhaseFlow_TPFA & operator=( SinglePhaseFlow_TPFA && ) = delete;

  /**
   * @brief default destructor
   */
  virtual ~SinglePhaseFlow_TPFA() = default;

  /**
   * @brief name of the node manager in the object catalog
   * @return string that contains the catalog name to generate a new NodeManager object through the object catalog.
   */
  static string CatalogName() { return "SinglePhaseFlow_TPFA"; }


  virtual void FillDocumentationNode() override final;

  virtual void FillOtherDocumentationNodes( dataRepository::ManagedGroup * const group ) override final;

  virtual void InitializeFinalLeaf( dataRepository::ManagedGroup * const problemManager ) override final;

  virtual void TimeStep( real64 const& time_n,
                         real64 const& dt,
                         integer const cycleNumber,
                         dataRepository::ManagedGroup * domain ) override;

  /**
   * @brief function to perform explicit time integration
   * @param time_n the time at the beginning of the step
   * @param dt the desired timestep
   * @param cycleNumber the current cycle number of the simulation
   * @param domain the domain partition
   */
  void TimeStepExplicit( real64 const& time_n,
                         real64 const& dt,
                         integer const cycleNumber,
                         DomainPartition * domain );

  /**
   * @brief function to perform quasi-static timestep
   * @param time_n the time at the beginning of the step
   * @param dt the desired timestep
   * @param cycleNumber the current cycle number of the simulation
   * @param domain the domain partition
   */
  void TimeStepQuasiStatic( real64 const& time_n,
                            real64 const& dt,
                            integer const cycleNumber,
                            DomainPartition& domain );


  /**
   * @brief function to perform setup for implicit timestep
   * @param time_n the time at the beginning of the step
   * @param dt the desired timestep
   * @param domain the domain partition
   */
  void ImplicitStepSetup( real64 const& time_n,
                              real64 const& dt,
                              DomainPartition * const domain ) override;

  /**
   * @brief function to perform cleanup for implicit timestep
   * @param time_n the time at the beginning of the step
   * @param dt the desired timestep
   * @param domain the domain partition
   */
  void ImplicitStepComplete( real64 const & time,
                                 real64 const & dt,
                                 DomainPartition * const domain ) override;

  /**
   * @brief This function sets the local and global rows, and calls functions to build linear system
   *        objects.
   * @param domain the domain partition
   * @param blockSystem
   *
   *
   */
  void SetupSystem ( DomainPartition * const domain,
                     systemSolverInterface::EpetraBlockSystem * const blockSystem );

  /**
   * @brief set the sparsity pattern for the linear system
   * @param domain the domain partition
   * @param sparsity the sparsity pattern matrix
   */
  void SetSparsityPattern( DomainPartition const * const domain,
                           Epetra_FECrsGraph * const sparsity );

  /**
   * @brief sets the dof indices for this solver
   * @param meshLevel the mesh object (single level only)
   * @param numLocalRows the number of local rows on this partition
   * @param numGlobalRows the number of global rows in the problem
   * @param localIndices unused TODO delete
   * @param offset the DOF offset for this solver in the case of a non-block system
   *
   * This function sets the number of global rows, and sets the dof numbers for
   * this solver. dof numbers are referred to trilinosIndices currently.
   */
  void SetNumRowsAndTrilinosIndices( MeshLevel * const meshLevel,
                                     localIndex & numLocalRows,
                                     localIndex & numGlobalRows,
                                     localIndex_array& localIndices,
                                     localIndex offset );


  real64 Assemble ( DomainPartition * const domain,
                    systemSolverInterface::EpetraBlockSystem * const blockSystem,
                    real64 const time,
                    real64 const dt ) override;


  /**
   * @brief Function to perform the Application of Dirichlet type BC's
   * @param object the target ObjectManager for the application of the BC.
   * @param bc the
   * @param set the set
   * @param time_n the time at the beginning of the step
   * @param blockSystem the entire block system
   */
  void ApplyDirichletBC_implicit( ManagedGroup * object,
                                  real64 const time,
                                  systemSolverInterface::EpetraBlockSystem & blockSystem);

  /**
   * @brief Function to apply the solution vector back onto the data fields
   * @param blockSystem the entire block system
   * @param scalingFactor factor to scale the solution prior to application
   * @param dofOffset the offset to the DOF indices
   * @param objectManager the object manager that holds the fields we wish to apply the solution to
   */
  void ApplySystemSolution( systemSolverInterface::EpetraBlockSystem const * const blockSystem,
                            real64 const scalingFactor,
                            localIndex const dofOffset,
                            DomainPartition * const domain ) override;

  /**
   * @brief This function generates various geometric information for later use.
   * @param domain the domain parition
   */
  void MakeGeometryParameters( DomainPartition * const  domain );


  /**
   * @enum an enum to lay out the time integration options.
   */
  enum class timeIntegrationOption
  {
    SteadyState,      //!< SteadyState
    ImplicitTransient,//!< ImplicitTransient
    ExplicitTransient //!< ExplicitTransient
  };

  struct viewKeyStruct : SolverBase::viewKeyStruct
  {
    constexpr static auto deltaFluidDensityString = "deltaFluidDensity";
    constexpr static auto deltaFluidPressureString = "deltaFluidPressure";
    constexpr static auto deltaPorosityString = "deltaPorosity";
    constexpr static auto deltaVolumeString = "deltaVolume";
    constexpr static auto faceAreaString = "faceArea";
    constexpr static auto faceCenterString = "faceCenter";
    constexpr static auto fluidPressureString = "fluidPressure";
    constexpr static auto gravityFlagString = "gravityFlag";
    constexpr static auto gravityForceString = "gravityForce";
    constexpr static auto permeabilityString = "permeablity";
    constexpr static auto porosityString = "porosity";
    constexpr static auto trilinosIndexString = "trilinosIndex_SinglePhaseFlow_TPFA";
    constexpr static auto volumeString = "volume";

    dataRepository::ViewKey trilinosIndex = { trilinosIndexString };
    dataRepository::ViewKey timeIntegrationOption = { "timeIntegrationOption" };
    dataRepository::ViewKey fieldVarName = { "fieldName" };
    dataRepository::ViewKey functionalSpace = { "functionalSpace" };
    dataRepository::ViewKey permeability = { permeabilityString };
  } viewKeys;

  struct groupKeyStruct : SolverBase::groupKeyStruct
  {
  } groupKeys;



private:
  /// the currently selected time integration option
  timeIntegrationOption m_timeIntegrationOption;

  /// temp variable containing distance between the face and element centers divided by the area of
  /// the face
  Array2dT<real64> m_faceToElemLOverA;

  /// the number of degrees of freedom per element. should be removed.
  constexpr static int m_dim = 1;

  /// temp array that holds the list of faces that connect two elements.
  localIndex_array m_faceConnectors;

  /// flag to determine whether or not to apply gravity
  integer m_gravityFlag;

  array< real64 > m_gravityForce;

  array< array< array<real64> > > m_dRho_dP;


};


} /* namespace geosx */

#endif /*  */