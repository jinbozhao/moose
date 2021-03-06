/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSINTEGRATEDBC_H
#define NSINTEGRATEDBC_H

#include "IntegratedBC.h"

// Forward Declarations
class NSIntegratedBC;

template<>
InputParameters validParams<NSIntegratedBC>();

/**
 * This class couples together all the variables
 * for the compressible Navier-Stokes equations to
 * allow them to be used in derived IntegratedBC
 * classes.  This prevents duplication of lines
 * of code between e.g. the momentum and energy
 * equations, since they have a lot in common.
 */
class NSIntegratedBC : public IntegratedBC
{
public:
  NSIntegratedBC(const std::string & name, InputParameters parameters);

  virtual ~NSIntegratedBC(){}

protected:
  /**
   * Not defined here, need to be defined in derived classes.
   */
  // virtual Real computeQpResidual();
  // virtual Real computeQpJacobian();
  // virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  VariableValue& _rho;
  VariableValue& _rho_u;
  VariableValue& _rho_v;
  VariableValue& _rho_w;
  VariableValue& _rho_e;

  // Gradients
  VariableGradient& _grad_rho;
  VariableGradient& _grad_rho_u;
  VariableGradient& _grad_rho_v;
  VariableGradient& _grad_rho_w;
  VariableGradient& _grad_rho_e;

  // Variable numberings
  unsigned _rho_var_number;
  unsigned _rhou_var_number;
  unsigned _rhov_var_number;
  unsigned _rhow_var_number;
  unsigned _rhoe_var_number;

  // Integrated BC can use Mat. properties...
  const MaterialProperty<Real> & _dynamic_viscosity;
  const MaterialProperty<RealTensorValue> & _viscous_stress_tensor; // Includes _dynamic_viscosity

  // Required parameters
  Real _gamma;
  Real _R;

  // Helper function for mapping Moose variable numberings into
  // the "canonical" numbering for the compressible NS equations.
  unsigned map_var_number(unsigned var);
};


#endif // NSINTEGRATEDBC_H
