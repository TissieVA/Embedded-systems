library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_unsigned.all;

entity TB_I2CIntf is
end TB_I2CIntf;

architecture Behav of TB_I2CIntf is
  constant c_ClkPeriod  : time := 10 ns; -- 100 MHz
  constant c_ClkHalfPeriod : time := c_ClkPeriod / 2;

  signal Clk            : std_logic := '0';
  signal ResetN         : std_logic;
  signal SDA            : std_logic;
  signal SCL            : std_logic;
  signal Leds           : std_logic_vector(7 downto 0);

begin
  
  Clk    <= not Clk after c_ClkHalfPeriod;
  ResetN <= '0', '1' after 3*c_ClkPeriod;
  
  Inst_I2CIntf: entity work.I2CIntf
    generic map(
      g_ClkFreq   => 100_000_000,
      g_BusFreq   => 100_000)
    port map(
      Clk         => Clk,
      ResetN      => ResetN,
      SDA         => SDA,
      SCL         => SCL,
      Leds        => Leds
      );
  
end Behav;
