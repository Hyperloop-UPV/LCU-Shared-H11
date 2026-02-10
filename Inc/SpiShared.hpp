#ifndef SPISHARED_HPP
#define SPISHARED_HPP

#include "HALAL/Models/SPI/SPIConfig.hpp"

auto constexpr spi_conf = []{
    ST_LIB::SPIConfigTypes::SPIConfig c;
    c.nss_mode = ST_LIB::SPIConfigTypes::NSSMode::SOFTWARE;
    return c;
}(); // Because I don't wanna write the entire constructor

#endif // SPISHARED_HPP