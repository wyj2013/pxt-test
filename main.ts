/**
 * Functions are mapped to blocks using various macros 
 * in comments starting with % (e.g., //% block).
 * The most important macro "block" specifies that a
 * block should be generated for a **exported** function.
 */
//% block="test" color="#88ccff"
namespace test {

    //% block
    export function helloWorld() {

    }

    // note that Caml casing yields lower case
    // block text with spaces
    //% weight=20
    //% blockID=analog_writepin
    //% block="analog output|%tpins|value|%value"
    //% tpins.fieldEditor="gridpicker"
    //% tpins.fieldOptins.columns=3
    export function analog_writepin(tpins: AnalogPin, value:number) :void{
        pins.analogWritePin(tpins,tvalue)
    }
    //% weight=30
    //% blockID=digital_writepin
    //% block="digital write pin|%tname| to |% tvalue"
    //% tpins.fieldEditor="gridpicker"
    //% tpins.fieldOptins.columns=3
    //% tvalue.defl=0
    //% tvalue.min=0
    //% tvalue.max=1
    export function digital_writepin(tpins: DigitalPin, tvalue:number) :void{
        pins.digitalWritePin(tname,tvalue)
    }
}
