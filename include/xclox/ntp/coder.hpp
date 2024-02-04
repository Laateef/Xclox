/*
 * Copyright (c) 2024 Abdullatif Kalla.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE.txt file in the root directory of this source tree.
 */

#ifndef XCLOX_CODER_HPP
#define XCLOX_CODER_HPP

namespace xclox {

namespace ntp {

    /**
     * @class Coder
     *
     * Coder is a data serializer and deserializer in big-endian format.
     *
     * @see The unit tests in @ref coder.h for further details.
     */
    class Coder {
    public:
        /**
         * Deserializes an integer from a raw data buffer.
         * @tparam Output is \e uint8_t, \e uint16_t, \e uint32_t, or \e uint64_t.
         * @param input is a pointer to the first byte of the data buffer to read from.
         * @return an integer of type \e Output.
         */
        template <typename Output>
        static Output deserialize(const uint8_t* input)
        {
            Output output { *input };
            for (size_t i = 1; i < sizeof(Output); ++i) {
                output = static_cast<Output>(output << 8 | *(input + i));
            }
            return output;
        }

        /**
         * Serializes an integer to a raw data buffer.
         * @tparam Input is \e uint8_t, \e uint16_t, \e uint32_t, or \e uint64_t.
         * @param input an integer to be serialized.
         * @param output a pointer to the first byte of the data buffer to write to.
         */
        template <typename Input>
        static void serialize(Input input, uint8_t* output)
        {
            for (size_t i = 0; i < sizeof(Input); ++i) {
                *(output + i) = static_cast<uint8_t>(input >> 8 * (sizeof(Input) - i - 1));
            }
        }
    };

} // namespace ntp

} // namespace xclox

#endif // XCLOX_CODER_HPP
