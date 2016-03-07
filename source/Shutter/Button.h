class Button {
	byte pin;
	word pause;
	unsigned long time;
	unsigned long start;
	boolean state;
	boolean is_down;
        boolean is_long;
	boolean is_up;
public:

	Button(byte pin) {
		this->pin = pin;
		pinMode(pin,INPUT);
		digitalWrite(pin,HIGH);
		time = millis();
		start = millis();
		state = false;
		is_down = false;
		is_up = false;
		pause = 100;
	}

	boolean pressed() {
		return state;
	}

	boolean down() {
		return is_down;
	}

        boolean is_long_pressed() {
                return is_long && state;
        }
        
        boolean is_long_down() {
                return is_long && state;
        }

	void update() {
		boolean old = state;
		if (millis() - time > 20) {
			state = digitalRead(pin) == LOW;
			time = millis();
		}
		is_down = state == true && old == false;
		is_up = state == false && old == true;
                if (is_up) is_long = 0;
		if (is_down) {
			start = time;
			pause = 1000;
                        is_long = 0;
		} else
			if (state && millis() - start > pause) {
				is_down = true;
				start += pause;
				pause = 250;
                                is_long = 1;
			}
	}

};
