/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiSmoothCircleIC.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<MultiSmoothCircleIC>()
{
  InputParameters params = validParams<SmoothCircleBaseIC>();
  params.addClassDescription("Random distribution of smooth circles with given minimum spacing");
  params.addRequiredParam<unsigned int>("numbub", "The number of bubbles to place");
  params.addRequiredParam<Real>("bubspac", "minimum spacing of bubbles, measured from center to center");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addParam<unsigned int>("numtries", 1000, "The number of tries");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circels");
  params.addParam<Real>("radius_variation", 0.0, "Plus or minus fraction of random variation in the bubble radius for uniform, standard deviation for normal");
  MooseEnum rand_options("uniform normal none","none");
  params.addParam<MooseEnum>("radius_variation_type", rand_options, "Type of distribution that random circle radii will follow");
  return params;
}

MultiSmoothCircleIC::MultiSmoothCircleIC(const std::string & name,
                                         InputParameters parameters) :
    SmoothCircleBaseIC(name, parameters),
    _numbub(getParam<unsigned int>("numbub")),
    _bubspac(getParam<Real>("bubspac")),
    _numtries(getParam<unsigned int>("numtries")),
    _radius(getParam<Real>("radius")),
    _radius_variation(getParam<Real>("radius_variation")),
    _radius_variation_type(getParam<MooseEnum>("radius_variation_type"))
{
  MooseRandom::seed(getParam<unsigned int>("rand_seed"));
}

void
MultiSmoothCircleIC::initialSetup()
{

  //Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  switch (_radius_variation_type)
  {
  case 2: //No variation
    if (_radius_variation > 0.0)
      mooseError("If radius_variation > 0.0, you must pass in a radius_variation_type in MultiSmoothCircleIC");
    break;
  }

  SmoothCircleBaseIC::initialSetup();
}

void
MultiSmoothCircleIC::computeCircleRadii()
{
  _radii.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary bubble radius
    switch (_radius_variation_type)
    {
    case 0: //Uniform distrubtion
      _radii[i] = _radius*(1.0 + (1.0 - 2.0*MooseRandom::rand())*_radius_variation);
      break;
    case 1: //Normal distribution
      _radii[i] = MooseRandom::randNormal(_radius,_radius_variation);
      break;
    case 2: //No variation
      _radii[i] = _radius;
    }

    if (_radii[i] < 0.0) _radii[i] = 0.0;
  }
}


void
MultiSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    //Vary circle center positions
    unsigned int num_tries = 0;

    Real rr = 0.0;
    Point newcenter = 0.0;

    while (rr < _bubspac && num_tries < _numtries)
    {
      num_tries++;
      //Moose::out<<"num_tries: "<<num_tries<<std::endl;

      Real ran1 = MooseRandom::rand();
      Real ran2 = MooseRandom::rand();
      Real ran3 = MooseRandom::rand();

      newcenter(0) = _bottom_left(0) + ran1*_range(0);
      newcenter(1) = _bottom_left(1) + ran2*_range(1);
      newcenter(2) = _bottom_left(2) + ran3*_range(2);

      for (unsigned int j = 0; j < i; j++)
      {
        if (j == 0) rr = _range.size();

        Real tmp_rr = _mesh.minPeriodicDistance(_var.number(), _centers[j], newcenter);

        if (tmp_rr < rr) rr = tmp_rr;
      }

      if (i == 0) rr = _range.size();
    }

    if (num_tries == _numtries)
      mooseError("Too many tries in MultiSmoothCircleIC");

    _centers[i] = newcenter;
  }
}
