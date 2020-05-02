/**
 * Functions are mapped to blocks using various macros 
 * in comments starting with % (e.g., //% block).
 * The most important macro "block" specifies that a
 * block should be generated for a **exported** function.
 */
//% block="test" color="#88ccff"
namespace test {
  
    //% weight=20
    //% blockId=digital_writepin 
    //% block="digital write pin |%tname| to |%tvalue"
    //% tname.fieldEditor="gridpicker" 
    //% tname.fieldOptions.columns=3 
    //% tvalue.defl=0
    //% tvalue.min=0
    //% tvalue.max=1	
    export function digital_writepin(tname: DigitalPin, tvalue: number): void {
        pins.digitalWritePin(tname, tvalue)
    }
    
    //% blockID=analog_writepin
    //% block="analog output|%tpins| value |%value"
    //% tpins.fieldEditor="gridpicker"
    //% tpins.fieldOptins.columns=3
    //% value.defl=0
    //% value.min=0
    //% value.max=255	
    export function analog_writepin(tpins: AnalogPin, value:number) :void{
        pins.analogWritePin(tpins,value)
    }

    
}
