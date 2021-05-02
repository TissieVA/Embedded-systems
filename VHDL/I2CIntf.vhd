library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity I2CIntf is
  generic(
    g_ClkFreq   : integer := 100_000_000;       -- input clock speed from user logic in Hz
    g_BusFreq   : integer := 100_000);          -- speed the i2c bus (scl) will run at in Hz
  port(
    Clk         : in std_logic;                 -- system clock
    ResetN      : in std_logic;                 -- active low reset
    SDA         : inout std_logic;              -- serial data output of i2c bus
    SCL         : inout std_logic;              -- serial clock output of i2c bus
    Leds        : out std_logic_vector(7 downto 0)); -- led outputs
end I2CIntf;

architecture RTL of I2CIntf is

 

  constant c_ClkDiv     : integer := g_ClkFreq / (4 * g_BusFreq); -- 1/4 of i2c bus frequency

  --- FSM for transferring one byte: START, SLAVE ADDRESS, ACK, DATA, ACK, STOP
  type t_BusState is (e_Idle, e_Start, e_Addr, e_AcknowledgeA, e_Data, e_AcknowledgeD, e_Stop);
  --- FSM for the i2c device: SET MODE, READ STATUS, if STATUS OK: READ 6 DATA REGS
  --- you might want to change this, depending on your device
  type t_Program is (e_Idle, e_SetMode, e_ReadStat, e_ReadData);
  type t_DataRead is array (0 to 5) of std_logic_vector(7 downto 0);

  signal BusState       : t_BusState;
  signal ClkDivCnt      : integer range 0 to c_ClkDiv*4;
  signal SclEn          : boolean;
  signal SdaInt         : std_logic;
  signal SdaEn          : boolean;

  constant c_SlaveAddr  : std_logic_vector(6 downto 0) := "1110111";
  signal Addr           : std_logic_vector(7 downto 0);
  signal Data           : std_logic_vector(7 downto 0);
  signal Program        : t_Program;
  signal DataRead       : t_DataRead;
  signal BitCnt         : integer range 0 to 8;
  signal ReadCnt        : integer range 0 to 5;
  signal Writing        : boolean;
  signal Reading        : boolean;
  signal WritingAddr    : boolean;
  signal Burst          : boolean;
  signal Status         : boolean;
  
  
--  attribute MARK_DEBUG : string;
--  attribute MARK_DEBUG of SDA: signal is "TRUE";
--  attribute MARK_DEBUG of SCL: signal is "TRUE";
--  attribute MARK_DEBUG of BusState: signal is "TRUE";
--  attribute MARK_DEBUG of Addr: signal is "TRUE";
--  attribute MARK_DEBUG of Data: signal is "TRUE";
--  attribute MARK_DEBUG of BitCnt: signal is "TRUE";
--  attribute MARK_DEBUG of SdaInt: signal is "TRUE";
  --attribute MARK_DEBUG of DataRead: signal is "TRUE";


begin
  
  ----------------------------------------------------------------
  -- bit timing:
  ----------------------------------------------------------------
  --       +-+ +-+ +-+ +-+ +-
  --       | | | | | | | | |
  --      -+ +-+ +-+ +-+ +-+
  --      (0) (1) (2) (3) (0)
  --      -+       +-------+
  --  SCL  |       |       |
  --       +-------+       +-
  -- 
  --      ----\ /------------
  --  SDA      X     stable
  --      ----/ \------------
  --
  p_BitTiming: process (Clk, ResetN)
  begin
    if ResetN = '0' then
      ClkDivCnt <= 0;
      SCL       <= 'Z';
      SDA       <= 'Z';
    elsif Clk'event and Clk = '1' then
      if ClkDivCnt = c_ClkDiv*4 -1 then
        ClkDivCnt       <= 0;
      else
        ClkDivCnt       <= ClkDivCnt + 1;
      end if;
      if ClkDivCnt = c_ClkDiv-1 then -- first 1/4 of the cycle (1 in bit timing above)
        if (SdaEn and SdaInt = '0') or BusState = e_Start or BusState = e_Stop then
          SDA <= '0';
        else
          SDA <= 'Z';
        end if;
      elsif ClkDivCnt = c_ClkDiv*2-1 then -- second 1/4 of the cycle (2 in bit timing above)
        if SclEn then
          SCL <= 'Z';
        end if;
      elsif ClkDivCnt = c_ClkDiv*3-1 then -- third 1/4 of the cycle (3 in bit timing above)
        if BusState = e_Stop then
          SDA <= 'Z';
        end if;
      elsif ClkDivCnt = c_ClkDiv*4-1 then -- end of the cycle (0 in bit timing above)
        if BusState /= e_Stop and SclEn then
          SCL <= '0';
        end if;
      end if;
    end if;
  end process p_BitTiming;
  
  ----------------------------------------------------------------
  -- byte timing:
  ----------------------------------------------------------------
  -- There are 3 possibilities for access to the i2c slave address (SADDR):
  --
  -- 1) Writing data (DATAW) to register address (RADDR)
  --
  -- +-------+-------------+------+-------+------+-------+------+------+
  -- | Start | SADDR+write | ACKA | RADDR | ACKD | DATAW | ACKD | Stop |
  -- +-------+-------------+------+-------+------+-------+------+------+
  --
  -- 2) Reading data (DATAR) from register address (RADDR)
  --
  -- +-------+-------------+------+-------+------+------+-------+------------+------+-------+-------+------+
  -- | Start | SADDR+write | ACKA | RADDR | ACKD | Stop | Start | SADDR+read | ACKA | DATAR | NACKD | Stop |
  -- +-------+-------------+------+-------+------+------+-------+------------+------+-------+-------+------+
  --
  -- 2) Burst reading data (DATAR1,2,...) from register address (RADDR)
  --
  -- +-------+-------------+------+-------+------+------+-------+------------+------+--------+------+--------+------+-----+-------+-------+------+
  -- | Start | SADDR+write | ACKA | RADDR | ACKD | Stop | Start | SADDR+read | ACKA | DATAR1 | ACKD | DATAR2 | ACKD | ... | DATARX| NACKD | Stop |
  -- +-------+-------------+------+-------+------+------+-------+------------+------+--------+------+--------+------+-----+-------+-------+------+
  --
  p_ByteTiming: process (Clk, ResetN)
  begin
    if ResetN = '0' then
      SclEn     <= false;
      SdaEn     <= false;
      SdaInt    <= '0';
      BusState  <= e_Idle;
      BitCnt    <= 0;
      DataRead  <= (others => (others => '0'));
      ReadCnt   <= 0;
      WritingAddr <= false;
    elsif Clk'event and Clk = '1' then -- same as rising edge
      if ClkDivCnt = c_ClkDiv*4-1 then --c_clockdiv =250; when clock has done 4*250-1 = 999 dan volledige SCL cycle geweest  
        if BitCnt > 0 then
          BitCnt <= BitCnt - 1;
        end if;
        case BusState is
          when e_Start =>
            BusState    <= e_Addr;
            BitCnt      <= 7;
            SdaInt      <= c_SlaveAddr(6);
            SclEn       <= true;
            SdaEn       <= true;
          when e_Addr =>
            SclEn       <= true;
            SdaEn       <= true;
            if BitCnt = 1 then
              if not WritingAddr and Reading then
                SdaInt  <= '1'; -- i2c read
              elsif Writing or WritingAddr then
                SdaInt  <= '0'; -- i2c write
              end if;
            elsif BitCnt = 0 then
              BusState  <= e_AcknowledgeA;
              SdaEn     <= false;
            else
              SdaInt      <= c_SlaveAddr(BitCnt-2); --#### moet bitcnt-2 zijn
            end if;
          when e_AcknowledgeA =>
            BusState    <= e_Data;
            SclEn       <= true;
            BitCnt      <= 7;     
            if Writing or (Reading and WritingAddr) then
              SdaInt    <= Addr(7);
              SdaEn     <= true;
            else
              SdaEn     <= false;
            end if;
          when e_Data =>
            if not WritingAddr and Reading then
              if SDA = '0' then
                DataRead(ReadCnt)(BitCnt) <= '0';                                           --######fout stond op 7-BitCnt
              else
                DataRead(ReadCnt)(BitCnt) <= '1';
              end if;
            end if;
            if BitCnt = 0 then
              BusState  <= e_AcknowledgeD;
              -- this specific device has 6 data registers, so you might want to change this number "5", depending on your device
              -- if you even need burst reading on your device
              if Reading and not WritingAddr and Burst and ReadCnt < 5 then -- read continuous
                SdaEn   <= true;
                SdaInt  <= '0'; -- Acknowledge (ACK)
              else
                SdaEn   <= false; -- not acknowledge (NACK) OR slave must acknowledge (ACK)
              end if;
            else
              if WritingAddr then
                SdaEn   <= true;
                SdaInt  <= Addr(BitCnt-1);  -- ########## fout stond op 8-bitcnt
              elsif Writing then
                SdaEn   <= true;
                SdaInt  <= Data(BitCnt-1);
              else
                SdaEn   <= false;
              end if;
            end if;
          when e_AcknowledgeD =>
            if Writing then
              SdaEn             <= true;
              if WritingAddr then
                WritingAddr     <= false;
                BusState        <= e_Data;
                BitCnt          <= 7;
                SdaInt          <= Data(7);
              else
                BusState        <= e_Stop;
              end if;
            elsif Reading then
              if WritingAddr then
                BusState        <= e_Stop;
                SdaEn           <= true;
                ReadCnt         <= 0;
              else
                if Burst and ReadCnt < 5 then
                  SdaEn         <= false;
                  BusState      <= e_Data;
                  BitCnt        <= 7;
                  ReadCnt       <= ReadCnt + 1;
                else
                  SdaEn         <= true;
                  BusState      <= e_Stop;
                  ReadCnt       <= 0;
                end if;
              end if;
            end if;
          when e_Stop =>
            if Reading and WritingAddr then
              BusState          <= e_Start;
              WritingAddr       <= false;
              SclEn             <= true;
              SdaEn             <= true;
            else
              BusState          <= e_Idle;
              WritingAddr       <= true;
              SclEn             <= false;
              SdaEn             <= false;
            end if;
          when others => -- e_Idle
            WritingAddr         <= true;
            if Reading or Writing then
              BusState          <= e_Start;
              SclEn             <= true;
              SdaEn             <= true;
            end if;
        end case;
      end if;
    end if;
  end process p_ByteTiming;
  
  -- this "program" works for one specific device. You will have to change this for your specific device.
  p_Program: process (Clk, ResetN)
  begin
    if ResetN = '0' then
      Program   <= e_Idle;
      Addr      <= (others => '0');
      Data      <= (others => '0');
      Writing   <= false;
      Reading   <= false;
      Burst     <= false;
      Status    <= false;
    elsif Clk'event and Clk = '1' then
      if ClkDivCnt = c_ClkDiv * 4 - 3 then -- one clk cycle before the Program FSM updates
        if Status and Program = e_ReadStat then -- Program has just read the status register
          if DataRead(0)(0) = '0' then -- the RDY bit of the status register is not set
            Program     <= e_ReadStat; -- read the status register again
            Addr        <= x"F6";
            Writing     <= false;
            Reading     <= true;
            Burst       <= false;
            Status      <= false;
          end if;
        end if;
      end if;
      if ClkDivCnt = c_ClkDiv * 4 - 2 then -- one clk cycle before p_ByteTiming updates
        if BusState = e_Idle then
          case Program is
            when e_SetMode =>
              Program   <= e_ReadStat;
              Addr      <= x"F6"; -- this specific device has a "status" register at address 09
              Writing   <= false;
              Reading   <= true;
              Burst     <= false;
              Status    <= true;
            when e_ReadStat =>
              Program   <= e_ReadData;
              Addr      <= x"AA"; -- this specific device has "data" registers at address 03 through 08
              Writing   <= false;
              Reading   <= true;
              Burst     <= true;
            when e_ReadData =>
              Program   <= e_Idle;
              Writing   <= false;
              Reading   <= false;
              Burst     <= false;
            when others => -- e_Idle
              Program   <= e_SetMode;
              Addr      <= x"F4"; -- this specific device has a "setmode" register at address 02
              Data      <= x"2E"; -- single mode
              Writing   <= true;
              Reading   <= false;
              Burst     <= false;
          end case;
        end if;
      end if;
    end if;
  end process p_Program;
  
end RTL;