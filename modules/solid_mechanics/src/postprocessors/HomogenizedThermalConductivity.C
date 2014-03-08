#include "HomogenizedThermalConductivity.h"
#include "SymmElasticityTensor.h"

#include "SubProblem.h"

template<>
InputParameters validParams<HomogenizedThermalConductivity>()
{
  InputParameters params = validParams<ElementAverageValue>();
  params.addRequiredCoupledVar("temp_x", "solution in x");
  params.addCoupledVar("temp_y", "solution in y");
  params.addCoupledVar("temp_z", "solution in z");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction this pp acts in (0 for x, 1 for y, 2 for z)");
  return params;
}

HomogenizedThermalConductivity::HomogenizedThermalConductivity(const std::string & name, InputParameters parameters)
  :ElementAverageValue(name, parameters),
   _grad_temp_x(coupledGradient("temp_x")),
   _grad_temp_y(_subproblem.mesh().dimension() > 1 ? coupledGradient("temp_y") : _grad_zero),
   _grad_temp_z(_subproblem.mesh().dimension() == 3 ? coupledGradient("temp_z") : _grad_zero),
   _component(getParam<unsigned int>("component")),
   _thermal_conductivity(getMaterialProperty<Real>("thermal_conductivity")),
   _volume(0),
   _integral_value(0)
{
  if (_component > 2)
  {
    mooseError("Component must be 0, 1, or 2 in HomogenizedThermalConductivity");
  }
}

void
HomogenizedThermalConductivity::initialize()
{
  _integral_value = 0;
  _volume = 0;
}

void
HomogenizedThermalConductivity::execute()
{
  _integral_value += computeIntegral();
  _volume += _current_elem_volume;
}

Real
HomogenizedThermalConductivity::getValue()
{

  gatherSum(_integral_value);
  gatherSum(_volume);

  return (_integral_value/_volume);
}



void
HomogenizedThermalConductivity::threadJoin(const UserObject & y)
{
  const HomogenizedThermalConductivity & pps = dynamic_cast<const HomogenizedThermalConductivity &>(y);

  _integral_value += pps._integral_value;
  _volume += pps._volume;
}

Real
HomogenizedThermalConductivity::computeQpIntegral()
{
  Real value(1);
  if (_component == 0)
  {
    value += _grad_temp_x[_qp](0);
  }
  else if (_component == 1)
  {
    value += _grad_temp_y[_qp](1);
  }
  else
  {
    value += _grad_temp_z[_qp](2);
  }

  return _thermal_conductivity[_qp] * value;
}