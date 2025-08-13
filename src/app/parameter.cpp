#include "app/parameter.h"
//#include "app/parameter_types.h"

Parameter* Parameter::create(ParameterType type) {
  switch (type) {
    case PARAMETER_TYPE_FLOAT:
      return new ParameterFloat();
    case PARAMETER_TYPE_INT:
      return new ParameterInt();
    case PARAMETER_TYPE_BOOL:
      return new ParameterBool();
    case PARAMETER_TYPE_STRING:
      return new ParameterString();
    default:
      // todo add error
      return new ParameterFloat();
  }
}
