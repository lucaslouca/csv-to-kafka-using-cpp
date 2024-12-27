
#include "AbstractTransformer.h"

AbstractTransformer::AbstractTransformer() : m_column("") {}

AbstractTransformer::AbstractTransformer(std::string column) : m_column(column) {}

AbstractTransformer::~AbstractTransformer(){};