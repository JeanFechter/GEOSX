/*
 * ------------------------------------------------------------------------------------------------------------
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 * Copyright (c) 2018-2020 Lawrence Livermore National Security LLC
 * Copyright (c) 2018-2020 The Board of Trustees of the Leland Stanford Junior University
 * Copyright (c) 2018-2020 TotalEnergies
 * Copyright (c) 2019-     GEOSX Contributors
 * All rights reserved
 *
 * See top level LICENSE, COPYRIGHT, CONTRIBUTORS, NOTICE, and ACKNOWLEDGEMENTS files for details.
 * ------------------------------------------------------------------------------------------------------------
 */

/**
 * @file SolidMechanicsLagrangianFEM.hpp
 */

#ifndef GEOS_PHYSICSSOLVERS_SOLIDMECHANICS_SOLIDMECHANICSLAGRANGIANFEM_HPP_
#define GEOS_PHYSICSSOLVERS_SOLIDMECHANICS_SOLIDMECHANICSLAGRANGIANFEM_HPP_

#include "codingUtilities/EnumStrings.hpp"
#include "common/TimingMacros.hpp"
#include "kernels/SolidMechanicsLagrangianFEMKernels.hpp"
#include "mesh/MeshForLoopInterface.hpp"
#include "mesh/mpiCommunications/CommunicationTools.hpp"
#include "mesh/mpiCommunications/MPI_iCommData.hpp"
#include "physicsSolvers/SolverBase.hpp"
#include "physicsSolvers/fluidFlow/FlowSolverBase.hpp"

#include "physicsSolvers/solidMechanics/SolidMechanicsFields.hpp"

namespace geos
{

/**
 * @class SolidMechanicsLagrangianFEM
 *
 * This class implements a finite element solution to the equations of motion.
 */
class SolidMechanicsLagrangianFEM : public SolverBase
{
public:

  /// String used to form the solverName used to register single-physics solvers in CoupledSolver
  static string coupledSolverAttributePrefix() { return "solid"; }

  /**
   * @enum TimeIntegrationOption
   *
   * The options for time integration
   */
  enum class TimeIntegrationOption : integer
  {
    QuasiStatic,      //!< QuasiStatic
    ImplicitDynamic,  //!< ImplicitDynamic
    ExplicitDynamic   //!< ExplicitDynamic
  };

  /**
   * Constructor
   * @param name The name of the solver instance
   * @param parent the parent group of the solver
   */
  SolidMechanicsLagrangianFEM( const string & name,
                               Group * const parent );


  SolidMechanicsLagrangianFEM( SolidMechanicsLagrangianFEM const & ) = delete;
  SolidMechanicsLagrangianFEM( SolidMechanicsLagrangianFEM && ) = default;

  SolidMechanicsLagrangianFEM & operator=( SolidMechanicsLagrangianFEM const & ) = delete;
  SolidMechanicsLagrangianFEM & operator=( SolidMechanicsLagrangianFEM && ) = delete;

  /**
   * destructor
   */
  virtual ~SolidMechanicsLagrangianFEM() override;

  /**
   * @return The string that may be used to generate a new instance from the SolverBase::CatalogInterface::CatalogType
   */
  static string catalogName() { return "SolidMechanics_LagrangianFEM"; }

  virtual void initializePreSubGroups() override;

  virtual void registerDataOnMesh( Group & meshBodies ) override final;

  void updateIntrinsicNodalData( DomainPartition * const domain );


  /**
   * @defgroup Solver Interface Functions
   *
   * These functions provide the primary interface that is required for derived classes
   */
  /**@{*/
  virtual
  real64 solverStep( real64 const & time_n,
                     real64 const & dt,
                     integer const cycleNumber,
                     DomainPartition & domain ) override;

  virtual
  real64 explicitStep( real64 const & time_n,
                       real64 const & dt,
                       integer const cycleNumber,
                       DomainPartition & domain ) override;

  virtual void
  implicitStepSetup( real64 const & time_n,
                     real64 const & dt,
                     DomainPartition & domain ) override;

  virtual void
  setupDofs( DomainPartition const & domain,
             DofManager & dofManager ) const override;

  virtual void
  setupSystem( DomainPartition & domain,
               DofManager & dofManager,
               CRSMatrix< real64, globalIndex > & localMatrix,
               ParallelVector & rhs,
               ParallelVector & solution,
               bool const setSparsity = false ) override;

  virtual void
  assembleSystem( real64 const time,
                  real64 const dt,
                  DomainPartition & domain,
                  DofManager const & dofManager,
                  CRSMatrixView< real64, globalIndex const > const & localMatrix,
                  arrayView1d< real64 > const & localRhs ) override;

  virtual void
  applySystemSolution( DofManager const & dofManager,
                       arrayView1d< real64 const > const & localSolution,
                       real64 const scalingFactor,
                       real64 const dt,
                       DomainPartition & domain ) override;

  virtual void updateState( DomainPartition & domain ) override final
  {
    // There should be nothing to update
    GEOS_UNUSED_VAR( domain );
  };

  virtual void applyBoundaryConditions( real64 const time,
                                        real64 const dt,
                                        DomainPartition & domain,
                                        DofManager const & dofManager,
                                        CRSMatrixView< real64, globalIndex const > const & localMatrix,
                                        arrayView1d< real64 > const & localRhs ) override;

  virtual real64
  calculateResidualNorm( real64 const & time_n,
                         real64 const & dt,
                         DomainPartition const & domain,
                         DofManager const & dofManager,
                         arrayView1d< real64 const > const & localRhs ) override;

  virtual void resetStateToBeginningOfStep( DomainPartition & domain ) override;

  virtual void implicitStepComplete( real64 const & time,
                                     real64 const & dt,
                                     DomainPartition & domain ) override;

  /**@}*/


  template< typename CONSTITUTIVE_BASE,
            typename KERNEL_WRAPPER,
            typename ... PARAMS >
  void assemblyLaunch( DomainPartition & domain,
                       DofManager const & dofManager,
                       CRSMatrixView< real64, globalIndex const > const & localMatrix,
                       arrayView1d< real64 > const & localRhs,
                       real64 const dt,
                       PARAMS && ... params );


  template< typename ... PARAMS >
  real64 explicitKernelDispatch( MeshLevel & mesh,
                                 arrayView1d< string const > const & targetRegions,
                                 string const & finiteElementName,
                                 real64 const dt,
                                 std::string const & elementListName );

  /**
   * Applies displacement boundary conditions to the system for implicit time integration
   * @param time The time to use for any lookups associated with this BC
   * @param dofManager degree-of-freedom manager associated with the linear system
   * @param domain The DomainPartition.
   * @param matrix the system matrix
   * @param rhs the system right-hand side vector
   * @param solution the solution vector
   */
  void applyDisplacementBCImplicit( real64 const time,
                                    DofManager const & dofManager,
                                    DomainPartition & domain,
                                    CRSMatrixView< real64, globalIndex const > const & localMatrix,
                                    arrayView1d< real64 > const & localRhs );

  void applyTractionBC( real64 const time,
                        DofManager const & dofManager,
                        DomainPartition & domain,
                        arrayView1d< real64 > const & localRhs );

  void applyChomboPressure( DofManager const & dofManager,
                            DomainPartition & domain,
                            arrayView1d< real64 > const & localRhs );


  void applyContactConstraint( DofManager const & dofManager,
                               DomainPartition & domain,
                               CRSMatrixView< real64, globalIndex const > const & localMatrix,
                               arrayView1d< real64 > const & localRhs );

  virtual real64
  scalingForSystemSolution( DomainPartition const & domain,
                            DofManager const & dofManager,
                            arrayView1d< real64 const > const & localSolution ) override;

  void enableFixedStressPoromechanicsUpdate();

  struct viewKeyStruct : SolverBase::viewKeyStruct
  {
    static constexpr char const * cflFactorString() { return "cflFactor"; }
    static constexpr char const * newmarkGammaString() { return "newmarkGamma"; }
    static constexpr char const * newmarkBetaString() { return "newmarkBeta"; }
    static constexpr char const * massDampingString() { return "massDamping"; }
    static constexpr char const * stiffnessDampingString() { return "stiffnessDamping"; }
    static constexpr char const * timeIntegrationOptionString() { return "timeIntegrationOption"; }
    static constexpr char const * maxNumResolvesString() { return "maxNumResolves"; }
    static constexpr char const * strainTheoryString() { return "strainTheory"; }
    static constexpr char const * solidMaterialNamesString() { return "solidMaterialNames"; }
    static constexpr char const * contactRelationNameString() { return "contactRelationName"; }
    static constexpr char const * noContactRelationNameString() { return "NOCONTACT"; }
    static constexpr char const * maxForceString() { return "maxForce"; }
    static constexpr char const * elemsAttachedToSendOrReceiveNodesString() { return "elemsAttachedToSendOrReceiveNodes"; }
    static constexpr char const * elemsNotAttachedToSendOrReceiveNodesString() { return "elemsNotAttachedToSendOrReceiveNodes"; }

    static constexpr char const * sendOrReceiveNodesString() { return "sendOrReceiveNodes";}
    static constexpr char const * nonSendOrReceiveNodesString() { return "nonSendOrReceiveNodes";}
    static constexpr char const * targetNodesString() { return "targetNodes";}
    static constexpr char const * forceString() { return "Force";}

    dataRepository::ViewKey newmarkGamma = { newmarkGammaString() };
    dataRepository::ViewKey newmarkBeta = { newmarkBetaString() };
    dataRepository::ViewKey massDamping = { massDampingString() };
    dataRepository::ViewKey stiffnessDamping = { stiffnessDampingString() };
    dataRepository::ViewKey timeIntegrationOption = { timeIntegrationOptionString() };
  } solidMechanicsViewKeys;

  SortedArray< localIndex > & getElemsAttachedToSendOrReceiveNodes( ElementSubRegionBase & subRegion )
  {
    return subRegion.getReference< SortedArray< localIndex > >( viewKeyStruct::elemsAttachedToSendOrReceiveNodesString() );
  }

  SortedArray< localIndex > & getElemsNotAttachedToSendOrReceiveNodes( ElementSubRegionBase & subRegion )
  {
    return subRegion.getReference< SortedArray< localIndex > >( viewKeyStruct::elemsNotAttachedToSendOrReceiveNodesString() );
  }

  real64 & getMaxForce() { return m_maxForce; }

  arrayView1d< ParallelVector > const & getRigidBodyModes() const
  {
    return m_rigidBodyModes;
  }

  array1d< ParallelVector > & getRigidBodyModes()
  {
    return m_rigidBodyModes;
  }

protected:
  virtual void postProcessInput() override final;

  virtual void initializePostInitialConditionsPreSubGroups() override final;

  virtual void setConstitutiveNamesCallSuper( ElementSubRegionBase & subRegion ) const override;

  real64 m_newmarkGamma;
  real64 m_newmarkBeta;
  real64 m_massDamping;
  real64 m_stiffnessDamping;
  TimeIntegrationOption m_timeIntegrationOption;
  real64 m_maxForce = 0.0;
  integer m_maxNumResolves;
  integer m_strainTheory;
  string m_contactRelationName;
  MPI_iCommData m_iComm;
  bool m_isFixedStressPoromechanicsUpdate;

  /// Rigid body modes
  array1d< ParallelVector > m_rigidBodyModes;

private:
  virtual void setConstitutiveNames( ElementSubRegionBase & subRegion ) const override;

};

ENUM_STRINGS( SolidMechanicsLagrangianFEM::TimeIntegrationOption,
              "QuasiStatic",
              "ImplicitDynamic",
              "ExplicitDynamic" );

//**********************************************************************************************************************
//**********************************************************************************************************************
//**********************************************************************************************************************


template< typename CONSTITUTIVE_BASE,
          typename KERNEL_WRAPPER,
          typename ... PARAMS >
void SolidMechanicsLagrangianFEM::assemblyLaunch( DomainPartition & domain,
                                                  DofManager const & dofManager,
                                                  CRSMatrixView< real64, globalIndex const > const & localMatrix,
                                                  arrayView1d< real64 > const & localRhs,
                                                  real64 const dt,
                                                  PARAMS && ... params )
{
  GEOS_MARK_FUNCTION;

  forDiscretizationOnMeshTargets( domain.getMeshBodies(), [&] ( string const &,
                                                                MeshLevel & mesh,
                                                                arrayView1d< string const > const & regionNames )
  {
    NodeManager const & nodeManager = mesh.getNodeManager();

    string const dofKey = dofManager.getKey( fields::solidMechanics::totalDisplacement::key() );
    arrayView1d< globalIndex const > const & dofNumber = nodeManager.getReference< globalIndex_array >( dofKey );

    real64 const gravityVectorData[3] = LVARRAY_TENSOROPS_INIT_LOCAL_3( gravityVector() );

    KERNEL_WRAPPER kernelWrapper( dofNumber,
                                  dofManager.rankOffset(),
                                  localMatrix,
                                  localRhs,
                                  dt,
                                  gravityVectorData,
                                  std::forward< PARAMS >( params )... );

    if( m_isFixedStressPoromechanicsUpdate )
    {
      m_maxForce = finiteElement::
                     regionBasedKernelApplication< parallelDevicePolicy< >,
                                                   CONSTITUTIVE_BASE,
                                                   CellElementSubRegion >( mesh,
                                                                           regionNames,
                                                                           this->getDiscretizationName(),
                                                                           FlowSolverBase::viewKeyStruct::solidNamesString(),
                                                                           kernelWrapper );
    }
    else
    {
      m_maxForce = finiteElement::
                     regionBasedKernelApplication< parallelDevicePolicy< >,
                                                   CONSTITUTIVE_BASE,
                                                   CellElementSubRegion >( mesh,
                                                                           regionNames,
                                                                           this->getDiscretizationName(),
                                                                           viewKeyStruct::solidMaterialNamesString(),
                                                                           kernelWrapper );
    }
  } );

  applyContactConstraint( dofManager, domain, localMatrix, localRhs );
}

} /* namespace geos */

#endif /* GEOS_PHYSICSSOLVERS_SOLIDMECHANICS_SOLIDMECHANICSLAGRANGIANFEM_HPP_ */
