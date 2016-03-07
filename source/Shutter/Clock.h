  

class Clock {
	
	byte seconds;
	byte minutes;
	byte hours;
	byte is_tire;
	byte hour_sound;
	unsigned long start;
        long correction;
	
	void updateInternal() {
		is_tire = (micros() - start) / 500000 % 2 == 0;
		
		// seconds
		if ((unsigned long)(micros() - start) >= 1000000 + correction) {
			seconds++;
			start += 1000000 + correction;
		}
		
		// minutes
		if (seconds >= 60) {
			seconds -= 60;
			minutes++;
		}
		
		// hours
		if (minutes >= 60) {
			minutes -= 60;
			hours++;
			hour_sound = 1;
		}
		
		// days
		if (hours >= 24) {
			hours -= 24;
		}
		
	}

        void setTime() {
               start = micros();
        }
        
public:
	
	Clock() {
		seconds = 0;
		minutes = 0;
		hours = 0;
		is_tire = 0;
                correction = 20 - 278 - 49;
                // 15:48
		start = micros();
	}
	
	void update() {
             updateInternal();

	}
	
	byte getSeconds() const {
		return seconds;
	}
	
	void setSeconds(int sec) {
		seconds = (sec + 60) % 60;
                setTime();
	}
	
	byte getMinutes() const {
		return minutes;
	}
	
	void setMinutes(int min) {
		minutes = (min + 60) % 60;
                setTime();
	}
	
	byte getHours() const {
		return hours;
	}
	
	void setHours(int hour) {
		hours = (hour + 24) % 24;
                setTime();
	}
	
	byte isHourSound() {
		byte ret = hour_sound;
		hour_sound = 0;
		return ret;
	}
	
	byte isTire() const {
		return is_tire;
	}

        void restart() {
            start = micros();
        }
        
        long getCorrection() {
            return correction;
        }
	
        void setCorrection(long correction) {
           this->correction = correction;
        }
        
        long *getCorrectionPointer() {
            return &correction;
        }
};
