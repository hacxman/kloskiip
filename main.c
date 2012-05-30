#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#define nil 0
#define USART_BAUDRATE 19200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL))) - 1) 

const int TICKLEN = 10;

typedef int(*fun_t)(int);

int* fmap(fun_t fun, int *array) {
  int *_array = array;
  for (; *array != nil; array++) {
    *array = fun(*array);
  }
  return _array;
}

ISR(USART_RX_vect) {
  char rb;
  rb = UDR0; // Fetch the received byte value into the variable "ByteReceived"
  UDR0 = rb;
  //_delay_us(10);
}

char _obuf[32];
ISR(USART_TX_vect) {
  char *buf;
  while (buf != _obuf+sizeof(_obuf)) {
    if (*buf == '\0')
      break;
    UDR0 = *buf;
    buf++;
  }
  _obuf[0] = '\0';
}

void uart_putstr(const char *str)
{
  while (*str)
   {
     while (!(UCSR0A & (1 << UDRE0)))
       ;
     UDR0 = *str++;
   }/*/
  char first = *str++;
  while (*str)
  {
    uart_put_fifo(&tx_buffer, *str++);
  }
 
  UDR0 = first;*/
}

void init(void) {
  DDRB |= 0b1111;
  PORTB &= ~0b1111;  // turn relays off


  UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   // Turn on the transmission and reception circuitry
  UCSR0C |= (0 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01); // Use 8-bit character sizes

  UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
  UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

  UCSR0B |= (1 << RXCIE0) || (0 << TXCIE0); // Enable the USART Recieve Complete interrupt (USART_RXC)
  sei(); // Enable the Global Interrupt Enable flag so that interrupts can be processed 
}

struct timespec {
  int16_t MSC;
  int8_t SEC;// = 0;
  int8_t MIN;// = 0;
  int8_t HRS;// = 0;
  int8_t DAY;// = 0;
  int8_t WEE;// = 0;
};
struct timespec _ts;
int8_t timepsilon = 10;

struct timespec sectotime(int32_t t);
int32_t gettime();
int __ticks = 0;
void tick() {
  _ts = sectotime(gettime() + TICKLEN);
  __ticks++;
/*  _ts.MSC += TICKLEN;
  if (_ts.MSC >  9) {_ts.SEC++; _ts.MSC=0;}
  if (_ts.SEC > 59) {_ts.MIN++; _ts.SEC=0;}
  if (_ts.MIN > 59) {_ts.HRS++; _ts.MIN=0;}
  if (_ts.HRS > 23) {_ts.DAY++; _ts.HRS=0;}
  if (_ts.DAY >  7) {_ts.WEE++; _ts.DAY=0;}*/
}
 
int32_t timetosec(struct timespec _ts) {
  return _ts.SEC*10 + _ts.MIN*600 + _ts.HRS*600*60 +
         _ts.DAY*600*60*24 + _ts.WEE*600*60*24*7 + _ts.MSC;
}
int32_t gettime() {
  return timetosec(_ts);
}
struct timespec sectotime(int32_t t) {
/*  struct timespec o;
  o.SEC += t.SEC;
  o.MIN += o.SEC/60;
  o.SEC = o.SEC-60*(o.SEC/60);

  o.MIN += t.MIN;
  o.HRS += o.MIN/60;
  o.MIN = o.MIN-60*(o.MIN/60);

  o.HRS += t.HRS;
  o.DAY += o.HRS/24;
  o.HRS = o.HRS-24*(o.HRS/24);

  o.DAY += t.DAY;
  o.WEE += o.DAY/7;
  o.DAY = o.DAY-60*(o.DAY/60);*/

  struct timespec o;
  o.WEE = t/(600*60*24*7);
  t -= o.WEE*(600*60*24*7);

  o.DAY = t/(600*60*24);
  t -= o.DAY*(600*60*24);

  o.HRS = t/(600*60);
  t -= o.HRS*(600*60);

  o.MIN = t/(600);
  t -= o.MIN*(600);

  o.SEC = t/10;
  t -= o.SEC*10;
  
  o.MSC = t;

  return o;

}
struct timespec addtimes(struct timespec t1, struct timespec t2) {
  return sectotime(timetosec(t1) + timetosec(t2));
}

struct jobspec {
  int8_t e:1; //is running (exe)
  int8_t c:1; //1: each 'recu' time; 0: at 'time' time
  int8_t s:1; //is suspended
  struct timespec time;
  struct timespec recu;
  fun_t job;
};

int advance(int b) {
  static int a = 0;
  if (a == 0b1111) {
    a = 0;
  }
  PORTB = a;
  a++;
  return a;
};

int on(int x) {
  PORTB |= 0b1;
}

int off(int x) {
  PORTB &= ~0b1;
}

int lala(int l) {
  uart_putstr("lala");
}

struct jobspec jobs[] = {
  {
  .recu = {.MIN=1}, //{0,  0, 1, 0, 0, 0},
  .time = {.SEC=10}, //{0, 10, 0, 0, 0, 0},
  .e = 0,
  .c = 1,
  .s = 0,
  .job = &on
  },
  {
  .recu = {0,  0, 1, 0, 0, 0},
  .time = {0, 30, 0, 0, 0, 0},
  .e = 0,
  .c = 1,
  .s = 0,
  .job = &off
  },
  {
  .recu = {0,  0, 12, 0, 0, 0},
  .time = {0, 11, 0, 0, 0, 0},
  .e = 0,
  .c = 1,
  .s = 0,
  .job = &lala
  },

};
int jobcnt = sizeof(jobs)/sizeof(jobs[0]);

void init_checks() {
  // check if there should be an ongoing event
  if (gettime() > timetosec(jobs[0].time) &&
      gettime() < timetosec(jobs[1].time)) {
    on(0);
  } else {
    off(0);
  }

}

void check_list() {
  struct jobspec * job = jobs;
  for (; job-jobs != jobcnt; job++) {
    if ((!job->s) &&
        (gettime() > timetosec(job->time) - timepsilon) &&
        (gettime() < timetosec(job->time) + timepsilon)) {
      job->s = 1;  //suspended

      job->e = 1;  //job running

      job->job(0);  //invoke job

      job->e = 0;  //job stopped

      if (job->c) {
        job->time = addtimes(job->time, job->recu); /*sectotime(
            timetosec(job->time) +
            (timetosec(job->recu) - (gettime() - timetosec(job->time)))
        );*/
      }

    } else if (job->s &&
        ((gettime() < timetosec(job->time) - timepsilon) ||
        (gettime() > timetosec(job->time) + timepsilon))) {
      job->s = 0;  //runnable
    }

  }
}

int main(void) {
  init();
  init_checks();
  while (1) {
    _delay_ms(TICKLEN*100);
    tick();
    check_list();
//    advance(0);
  };
}
