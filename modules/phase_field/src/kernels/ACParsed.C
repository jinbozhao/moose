/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACParsed.h"

template<>
InputParameters validParams<ACParsed>()
{
  InputParameters params = DerivativeKernelInterface<ACBulk>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
  params.addCoupledVar("args", "Vector of additional arguments to F");
  return params;
}

ACParsed::ACParsed(const std::string & name, InputParameters parameters) :
    DerivativeKernelInterface<JvarMapInterface<ACBulk> >(name, parameters),
    _dFdEta(getMaterialPropertyDerivative<Real>(_F_name, _var.name())),
    _d2FdEta2(getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _var.name()))
{
  // reserve space for derivatives
  _d2FdEtadarg.resize(_nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
    _d2FdEtadarg[i] = &getMaterialPropertyDerivative<Real>(_F_name, _var.name(), _coupled_moose_vars[i]->name());
}

Real
ACParsed::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case Residual:
      return _dFdEta[_qp];

    case Jacobian:
      return _d2FdEta2[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}

Real
ACParsed::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  unsigned int cvar;
  if (!mapJvarToCvar(jvar, cvar))
    return 0.0;

  return _L[_qp] * (*_d2FdEtadarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
