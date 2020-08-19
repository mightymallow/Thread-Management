// Candy creation/consumption statistics
#ifndef STATS_H
#define STATS_H

void stats_init(int num_producers);
void stats_cleanup(void);
void stats_record_produced(int factory_number);
void stats_record_consumed(int producer_number, double delay_in_ms);
void stats_display(void);

#endif
