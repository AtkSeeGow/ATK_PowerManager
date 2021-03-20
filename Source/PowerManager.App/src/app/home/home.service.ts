import { Injectable } from '@angular/core';
import { HttpClient } from "@angular/common/http";

@Injectable()
export class HomeService {
  constructor(
    private http: HttpClient
  ) { };

  asyncReadDigitalValue() {
    return this.http.get<any>('./Api/Serial/ReadDigitalValue');
  };

  asyncSwitchOpticalCoupler(interval:number) {
    return this.http.get<any>(`./Api/Serial/SwitchOpticalCoupler?interval=${interval}`);
  };

  asyncStatus() {
    return this.http.get<any>(`./Api/Host/Status`);
  };
}
